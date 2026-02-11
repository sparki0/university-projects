using System;
using System.Collections.Generic;
using System.Data;
using System.IO;
using System.Linq;
using System.Net.Http.Headers;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace TCP_Server {
  internal class Program {
    static void Main(string[] args) {
      TcpListener server = new TcpListener(3999);
      server.Start();
      while(true) {
        Console.WriteLine("Listening for client.");
        TcpClient client = server.AcceptTcpClient();
        Task.Run(() => Logika(client));
      }

    }

    static void Logika(TcpClient client) {
      MyClient myClient = new(client);
      try {
        myClient.Work();
      } catch {

      } finally {
        myClient.Client.Close();
      }
    }
  }
  enum ChatState {
    RegState, LogState, MovingState, TextState
  }

  enum ErrorMessage {
    LoginError, SyntaxError, LogicError, KeyError
  }
  enum MovingMsg {
    Move, TurnRight, TurnLeft
  }
  enum Direction {
    UP, DOWN, LEFT, RIGHT
  }
  internal class Robot {
    public int X { get; set; }
    public int Y { get; set; }

    public int CrushCount { get; set; }

    public Direction Direction { get; set; }

    public Robot() {
      CrushCount = 0;
    }
    public void TurnLeft() {
      if(Direction.UP == Direction) {
        Direction = Direction.LEFT;
      } else if(Direction.DOWN == Direction) {
        Direction = Direction.RIGHT;
      } else if(Direction.LEFT == Direction) {
        Direction = Direction.DOWN;
      } else if(Direction.RIGHT == Direction) {
        Direction = Direction.UP;
      }
    }

    public void TurnRight() {
      if(Direction.UP == Direction) {
        Direction = Direction.RIGHT;
      } else if(Direction.DOWN == Direction) {
        Direction = Direction.LEFT;
      } else if(Direction.LEFT == Direction) {
        Direction = Direction.UP;
      } else if(Direction.RIGHT == Direction) {
        Direction = Direction.DOWN;
      }
    }

    public void TurnAround() {
      if(Direction.UP == Direction) {
        Direction = Direction.DOWN;
      } else if(Direction.DOWN == Direction) {
        Direction = Direction.UP;
      } else if(Direction.LEFT == Direction) {
        Direction = Direction.RIGHT;
      } else if(Direction.RIGHT == Direction) {
        Direction = Direction.LEFT;
      }
    }
  }
  class PairKey {
    public int ServerKey { get; set; }
    public int ClientKey { get; set; }
  }

  internal class MyClient {
    private static Dictionary<int,PairKey> _serverClientKeys = new Dictionary<int,PairKey>() {
      [0] = new PairKey() { ServerKey = 23019,ClientKey = 32037 },
      [1] = new PairKey() { ServerKey = 32037,ClientKey = 29295 },
      [2] = new PairKey() { ServerKey = 18789,ClientKey = 13603 },
      [3] = new PairKey() { ServerKey = 16443,ClientKey = 29533 },
      [4] = new PairKey() { ServerKey = 18189,ClientKey = 21952 }
    };
    StreamReader _reader;
    StreamWriter _writer;
    public TcpClient Client { get; set; }
    ChatState _chatState;
    Robot _robot;
    public MyClient(TcpClient client) {
      Client = client;
      _reader = new StreamReader(Client.GetStream());
      _writer = new StreamWriter(Client.GetStream());
      _robot = new Robot();
      Client.ReceiveTimeout = 1000;
    }

    public void Work() {
      if(!LoginState() || !MovingState()) {
        return;
      }
      GetConText();
    }

    private bool LoginState() {
      _chatState = ChatState.RegState;
      string msg = "";
      if(!GetMsgClient(out msg)) {
        return false;
      }

      return KeyRequest(msg);

    }
    private bool KeyRequest(string userName) {
      _chatState = ChatState.LogState;
      _writer.Write("107 KEY REQUEST\a\b");
      _writer.Flush();
      string msg = "";
      int keyId;

      if(!GetMsgClient(out msg) || msg.Length > 3) {
        return false;
      } else if(!int.TryParse(msg,out keyId)) {
        SendErrorMsg(ErrorMessage.SyntaxError);
        return false;
      } else if(keyId < 0 || keyId > 4) {
        SendErrorMsg(ErrorMessage.KeyError);
        return false;
      }

      int commKey = (userName.Sum(s => (int)s) * 1000) % 65536;
      int serverKey = (commKey + _serverClientKeys[keyId].ServerKey) % 65536;
      _writer.Write(serverKey.ToString() + "\a\b");
      _writer.Flush();

      int clientAnsKey;
      if(!GetMsgClient(out msg)) {
        return false;
      } else if(msg.Length > 5 || !int.TryParse(msg,out clientAnsKey)) {
        SendErrorMsg(ErrorMessage.SyntaxError);
        return false;
      }

      int clientKey = (commKey + _serverClientKeys[keyId].ClientKey) % 65536;
      if(clientKey != clientAnsKey) {
        SendErrorMsg(ErrorMessage.LoginError);
        return false;
      }

      _writer.Write("200 OK\a\b");
      _writer.Flush();

      return true;
    }

    private bool MovingState() {
      _chatState = ChatState.MovingState;
      if(!GetDirection() || !Move()) {
        return false;
      }

      return true;
    }

    private bool GetDirection() {
      int x = 0, y = 0;
      int newX = 0, newY = 0;
      string output;
      SendMoveMsg(MovingMsg.Move);

      if(!GetMsgClient(out output)) {
        return false;
      }

      if(!ParseCoorMsg(output,ref x,ref y)) {
        return false;
      }

      SendMoveMsg(MovingMsg.Move);

      while(true) {
        if(!GetMsgClient(out output)) {
          return false;
        }

        if(!ParseCoorMsg(output,ref newX,ref newY)) {
          return false;
        }

        if(x != newX || y != newY) {
          break;
        } else {
          _robot.CrushCount++;
          if(!TurnRight()) {
            return false;
          }
          SendMoveMsg(MovingMsg.Move);
        }

      }

      _robot.X = newX;
      _robot.Y = newY;

      if(y == newY) {
        _robot.Direction = newX < x ? Direction.LEFT : Direction.RIGHT;
      } else if(x == newX) {
        _robot.Direction = newY < y ? Direction.DOWN : Direction.UP;
      }

      return true;
    }

    private bool Move() {
      string output;
      int x = 0, y = 0;
      while(_robot.X != 0 || _robot.Y != 0) {
        if(Math.Abs(_robot.X) > Math.Abs(_robot.Y)) {
          if(!CheckAndChangeX()) { return false; };
        } else {
          if(!CheckAndChangeY()) { return false; };
        }
        if(!MakeMove(ref x,ref y)) {
          SendErrorMsg(ErrorMessage.SyntaxError);
          return false;
        }
        if(_robot.X == x && _robot.Y == y) {
          _robot.CrushCount++;
          if(_robot.CrushCount == 20 || !TurnRight()) {
            SendErrorMsg(ErrorMessage.SyntaxError);
            return false;
          }
          if(!MakeMove(ref x,ref y)) {
            SendErrorMsg(ErrorMessage.SyntaxError);
            return false;
          }
        } else {
          _robot.X = x;
          _robot.Y = y;
        }

      }

      return true;
    }



    private bool MakeMove(ref int x,ref int y) {
      string output;
      SendMoveMsg(MovingMsg.Move);
      if(!GetMsgClient(out output)) {
        return false;
      } else if(!ParseCoorMsg(output,ref x,ref y)) {
        return false;
      }


      return true;
    }

    private bool GetConText() {
      string output = "";
      _chatState = ChatState.TextState;
      _writer.Write("105 GET MESSAGE\a\b");
      _writer.Flush();
      if(!GetMsgClient(out output)) {
        return false;
      }

      _writer.Write("106 LOGOUT\a\b");
      _writer.Flush();


      return true;
    }


    private bool CheckAndChangeX() {
      if(_robot.X > 0) {
        if(_robot.Direction == Direction.RIGHT) {
          if(!TurnAround()) return false;
        } else if(_robot.Direction == Direction.UP) {
          if(!TurnLeft()) return false;
        } else if(_robot.Direction == Direction.DOWN) {
          if(!TurnRight()) return false;
        }
      } else {
        if(_robot.Direction == Direction.LEFT) {
          if(!TurnAround()) return false;
        } else if(_robot.Direction == Direction.UP) {
          if(!TurnRight()) return false;
        } else if(_robot.Direction == Direction.DOWN) {
          if(!TurnLeft()) return false;
        }

      }
      return true;
    }
    private bool CheckAndChangeY() {
      if(_robot.Y > 0) {
        if(_robot.Direction == Direction.UP) {
          if(!TurnAround()) return false;
        } else if(_robot.Direction == Direction.LEFT) {
          if(!TurnLeft()) return false;
        } else if(_robot.Direction == Direction.RIGHT) {
          if(!TurnRight()) return false;
        }
      } else {
        if(_robot.Direction == Direction.DOWN) {
          if(!TurnAround()) return false;
        } else if(_robot.Direction == Direction.LEFT) {
          if(!TurnRight()) return false;
        } else if(_robot.Direction == Direction.RIGHT) {
          if(!TurnLeft()) return false;
        }
      }
      return true;
    }


    private bool TurnAround() {
      string output;
      int x = 0, y = 0;

      for(int i = 0;i < 2;++i) {
        SendMoveMsg(MovingMsg.TurnRight);

        if(!GetMsgClient(out output)) {
          return false;
        }

        if(!ParseCoorMsg(output,ref x,ref y)) {
          return false;
        }
      }

      _robot.TurnAround();

      return true;
    }

    private bool TurnRight() {
      string output;
      int x = 0, y = 0;
      SendMoveMsg(MovingMsg.TurnRight);

      if(!GetMsgClient(out output)) {
        return false;
      }
      if(!ParseCoorMsg(output,ref x,ref y)) {
        return false;
      }

      _robot.TurnRight();

      return true;
    }

    private bool TurnLeft() {
      string output;
      int x = 0, y = 0;
      SendMoveMsg(MovingMsg.TurnLeft);

      if(!GetMsgClient(out output)) {
        return false;
      }
      if(!ParseCoorMsg(output,ref x,ref y)) {
        return false;
      }
      _robot.TurnLeft();

      return true;
    }


    private bool ParseCoorMsg(string msg,ref int x,ref int y) {
      var tokens = msg.Split(' ');
      if(!int.TryParse(tokens[1],out x) || !int.TryParse(tokens[2],out y)) {
        SendErrorMsg(ErrorMessage.SyntaxError);
        return false;
      }

      return true;
    }

    private bool GetMsgClient(out string output) {
      output = "";
      if(_chatState == ChatState.RegState) {
        if(!Read(out output,20)) {
          SendErrorMsg(ErrorMessage.SyntaxError);
          return false;
        }
      } else if(_chatState == ChatState.LogState) {
        if(!Read(out output,12)) {
          SendErrorMsg(ErrorMessage.SyntaxError);
          return false;
        }
      } else if(_chatState == ChatState.MovingState) {
        if(!Read(out output,12)) {
          SendErrorMsg(ErrorMessage.SyntaxError);
          return false;
        }
      } else if(_chatState == ChatState.TextState) {
        if(!Read(out output,100)) {
          SendErrorMsg(ErrorMessage.SyntaxError);
          return false;
        }
      }
      if(output == "RECHARGING") {
        if(!Charging(ref output)) {
          return false;
        }
      }
      return true;
    }

    private bool Charging(ref string output) {
      output = "";
      Client.ReceiveTimeout = 5000;
      if(!Read(out output,12) || output != "FULL POWER") {
        SendErrorMsg(ErrorMessage.LogicError);
        return false;
      }
      Client.ReceiveTimeout = 1000;
      if(!Read(out output,_chatState == ChatState.TextState ? 100 : 12)) {
        SendErrorMsg(ErrorMessage.SyntaxError);

        return false;
      }
      return true;
    }

    private bool Read(out string output,int maxLen) {
      output = "";
      char c1;
      char c2 = ' ';
      for(int i = 0;;++i) {
        c1 = (char)_reader.Read();
        output += c1;
        if(output.Length >= maxLen) {
          return false;
        }

        if(c1 == '\a') {
          c2 = (char)_reader.Read();
          output += c2;
          if(c2 == '\b') {
            break;
          }
        }
        if((c1 == '\a' && _reader.Peek() == '\b')) {
          break;
        }
      }

      if(c2 != '\b') {
        output += (char)_reader.Read();
      }

      output = output.Substring(0,output.Length - 2);
      if(output.Last() == ' ') {
        return false;
      }

      Console.WriteLine($"Received msg: {output}");
      return true;
    }
    public void SendMoveMsg(MovingMsg moving) {
      if(moving == MovingMsg.Move) {
        _writer.Write("102 MOVE\a\b");
      } else if(moving == MovingMsg.TurnLeft) {
        _writer.Write("103 TURN LEFT\a\b");
      } else if(moving == MovingMsg.TurnRight) {
        _writer.Write("104 TURN RIGHT\a\b");
      }
      _writer.Flush();
    }
    private void SendErrorMsg(ErrorMessage msg) {
      if(msg == ErrorMessage.SyntaxError) {
        _writer.Write("301 SYNTAX ERROR\a\b");
      } else if(msg == ErrorMessage.LoginError) {
        _writer.Write("300 LOGIN FAILED\a\b");
      } else if(msg == ErrorMessage.LogicError) {
        _writer.Write("302 LOGIC ERROR\a\b");
      } else if(msg == ErrorMessage.KeyError) {
        _writer.Write("303 KEY OUT OF RANGE\a\b");
      }
      _writer.Flush();
    }

  }
}
