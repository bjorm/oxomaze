#include "OXOcardRunnerV2.h"

// courtesy of https://github.com/muwerk/ustd
#include <array.h>
#include <map.h>

enum Direction {
  UP, DOWN, LEFT, RIGHT
};

Direction directions[] = {UP, LEFT, RIGHT};

struct Position {
  int x;
  int y;
  Direction direction;

  Position() {
    this->x = 0;
    this->y = 0;
    this->direction = UP;
  }

  Position(int x, int y, Direction direction) {
    this->x = x;
    this->y = y;
    this->direction = direction;
  }
  bool operator==(const Position& a) const {
    return a.x == x && a.y == y;
  }
};

typedef ustd::map<Position, Position> Path;

bool would_hit_boundary(Direction direction, Position& current_position, int max_size) {
  bool would_hit_bottom = direction == DOWN && current_position.y == 0;
  bool would_hit_left = direction == LEFT && current_position.x == 0;
  bool would_hit_right = direction == RIGHT && current_position.x == max_size;
  return would_hit_right || would_hit_bottom || would_hit_left;
}

bool is_legal_move(Path& previous_directions, Direction next_direction) {
  int path_length = previous_directions.length();

  if (path_length < 2) {
    return true;
  }

  if (next_direction == UP) {
    return true;
  }

  if (previous_directions.keys[path_length - 1].direction == RIGHT && next_direction == LEFT ||
      previous_directions.keys[path_length - 1].direction == LEFT && next_direction == RIGHT) {
    return false;
  }

  if ((next_direction == RIGHT || next_direction == LEFT) &&
      previous_directions.keys[path_length - 1].direction == UP &&
      previous_directions.keys[path_length - 2].direction != UP) {
    return false;
  }

  return true;
}

Position compute_next_position(Direction next_direction, Position current_position) {
  Position new_position;
  new_position.direction = next_direction;
  switch (next_direction) {
    case UP:
      new_position.x = current_position.x;
      new_position.y = current_position.y + 1;
      break;
    case DOWN:
      new_position.x = current_position.x;
      new_position.y = current_position.y - 1;
      break;
    case LEFT:
      new_position.x = current_position.x - 1;
      new_position.y = current_position.y;
      break;
    case RIGHT:
      new_position.x = current_position.x + 1;
      new_position.y = current_position.y;
      break;
  }

  return new_position;
}

Direction get_next_direction() {
  return directions[random(3)];
}

Position get_next_position(Position current_position, Path& path, int max_size) {
  Position next_position;
  Direction next_direction;
  bool illegal, not_unique, outside_boundary;
  Serial.println("begin get_next_position");
  do {
    next_direction = get_next_direction();
    next_position = compute_next_position(next_direction, current_position);
    Serial.println("Next direction");
    Serial.println(next_position.x);
    Serial.println(next_position.y);
    illegal = !is_legal_move(path, next_direction);
    not_unique = path.find(next_position) != -1;
    outside_boundary = would_hit_boundary(next_direction, current_position, max_size);
    Serial.print("Illegal: ");
    Serial.println(illegal);
    Serial.print("not unique: ");
    Serial.println(not_unique);
    Serial.print("Outside boundary: ");
    Serial.println(outside_boundary);
  } while (illegal || not_unique || outside_boundary);

  Serial.println("return next_position");
  return next_position;
}

Path get_random_path(int board_size) {
  int max_size = board_size - 1;
  Path path;

  Position current_position = {random(max_size), 0, UP};
  path[current_position] = current_position;

  do {
    Position next_position = get_next_position(current_position, path, max_size);
    path[next_position] = next_position;
    current_position = next_position;
  } while (current_position.y != max_size);
  
  return path;
}

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));
}

void loop() {
  int board_size = 8;
  Serial.println("");

  Path random_path = get_random_path(board_size);

  oxocard.matrix->clearScreen();
  oxocard.matrix->setForeColor(Random::getColor());

  for (int i = 0; i < random_path.length(); i++ ) {
    Position step = random_path.keys[i];
    oxocard.matrix->drawPixel(step.x, step.y);
  }
  Serial.println("exit");
}