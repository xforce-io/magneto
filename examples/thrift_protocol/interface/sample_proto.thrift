namespace cpp app

struct Ping {
  1: required i64 token,
}

struct Pong {
  1: required i64 token,
}

service PingPongTest {
  Pong PingPong(1: Ping ping);
}
