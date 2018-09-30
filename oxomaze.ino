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

  bool operator==(const Position& position) const {
    return position.x == x && position.y == y;
  }
};

typedef ustd::map<Position, Position> Path;

bool would_hit_boundary(Direction direction, Position& current_position, int max_size) {
  bool would_hit_bottom = direction == DOWN && current_position.y == 0;
  bool would_hit_left = direction == LEFT && current_position.x == 0;
  bool would_hit_right = direction == RIGHT && current_position.x == max_size;
  return would_hit_right || would_hit_bottom || would_hit_left;
}

bool is_illegal_move(ustd::array<Direction>& previous_directions, Direction next_direction) {
  int path_length = previous_directions.length();

  if (path_length < 2) {
    return false;
  }

  if (next_direction == UP) {
    return false;
  }

  Direction last_direction = previous_directions[path_length - 1];

  if (last_direction == RIGHT && next_direction == LEFT ||
      last_direction == LEFT && next_direction == RIGHT) {
    return true;
  }

  Direction second_last_direction = previous_directions[path_length - 2];

  if ((next_direction == RIGHT || next_direction == LEFT) &&
      last_direction == UP && second_last_direction != UP) {
    return true;
  }

  return false;
}

Position compute_next_position(Direction next_direction, Position current_position) {
  Position new_position = {current_position.x, current_position.y};

  if (next_direction == UP) {
    new_position.y = new_position.y + 1;
  } else if (next_direction == DOWN) {
    new_position.y = new_position.y - 1;
  } else if (next_direction == LEFT) {
    new_position.x = new_position.x - 1;
  } else if (next_direction == RIGHT) {
    new_position.x = new_position.x + 1;
  }

  return new_position;
}

Direction get_next_direction() {
  return directions[random(3)];
}

Position get_next_position(Position current_position, Path& path, int max_size) {
  Position next_position;
  Direction next_direction;
  ustd::array<Direction> previous_directions;
  bool illegal, not_unique, outside_boundary;

  do {
    next_direction = get_next_direction();
    next_position = compute_next_position(next_direction, current_position);
    illegal = is_illegal_move(previous_directions, next_direction);
    not_unique = path.find(next_position) != -1;
    outside_boundary = would_hit_boundary(next_direction, current_position, max_size);
  } while (illegal || not_unique || outside_boundary);

  previous_directions.add(next_direction);
  return next_position;
}

Path get_random_path(int board_size, Position current_position) {
  int max_size = board_size - 1;
  Path path;

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

  Position start_position = {random(board_size - 1), 0};

  Path random_path = get_random_path(board_size, start_position);
  Path random_path2 = get_random_path(board_size, start_position);

  oxocard.matrix->clearScreen();
  oxocard.matrix->setForeColor(rgb(255, 0, 0));

  Position player_position = random_path.keys[0];
  oxocard.matrix->drawPixel(player_position.x, player_position.y);

  oxocard.matrix->setForeColor(rgb(0, 102, 0));

  for (int x = 0; x < board_size; x++ ) {
    for (int y = 0; y < board_size; y++ ) {
      Position pos = {x, y};
      if (random_path.find(pos) == -1 && random_path2.find(pos) == -1) {
        oxocard.matrix->drawPixel(pos.x, pos.y);
      }
    }
  }

  Serial.println("exit");
}