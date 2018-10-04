#include "OXOcardRunnerV2.h"

// Courtesy of https://github.com/muwerk/ustd
#include <array.h>
#include <map.h>

// Based on LIS3DE::orientation_t in LIS3DE.h
enum Direction {
  NEUTRAL = 1,
  UP = 6,
  DOWN = 5,
  LEFT = 7,
  RIGHT = 8
};

Direction directions[] = {UP, LEFT, RIGHT};
rgbColor_t RED = rgb(255, 0, 0);
rgbColor_t GREEN = rgb(0, 102, 0);

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

Direction get_tilt_direction() {
  int8_t orientation = oxocard.accelerometer->getOrientation();
  if (orientation == (int)UP) {
    return UP;
  } else if (orientation == (int)DOWN) {
    return DOWN;
  } else if (orientation == (int)RIGHT) {
    return RIGHT;
  } else if (orientation == (int)LEFT) {
    return LEFT;
  } else {
    return NEUTRAL;
  }
}

Position move_player(Position current_position, Direction direction) {
  oxocard.matrix->clearPixel(current_position.x, current_position.y);
  Position new_position = compute_next_position(direction, current_position);
  oxocard.matrix->drawPixel(new_position.x, new_position.y, RED);
  return new_position;
}

bool hits_wall(Position position) {
  rgbColor_t color = oxocard.matrix->readPixel(position.x, position.y);
  return color.r == GREEN.r && color.g == GREEN.g && color.b == GREEN.b;
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

  Position player_position = random_path.keys[0];
  move_player(player_position, NEUTRAL);

  for (int x = 0; x < board_size; x++ ) {
    for (int y = 0; y < board_size; y++ ) {
      Position pos = {x, y};
      if (random_path.find(pos) == -1 && random_path2.find(pos) == -1) {
        oxocard.matrix->drawPixel(pos.x, pos.y, GREEN);
      }
    }
  }

  const long INTERVAL_MILLIS = 600;
  unsigned long last_millis = 0;
  unsigned long current_millis = 0;

  while (true) {
    current_millis = millis();

    if (current_millis - last_millis > INTERVAL_MILLIS) {
      Direction player_direction = get_tilt_direction();
      Position next_position = compute_next_position(player_direction, player_position);

      if (hits_wall(next_position)) {
        oxocard.matrix->clearScreen();
        oxocard.matrix->drawCharBigFont('X', 2, 0, RED);
        delay(2000);
        break;
      }

      if (next_position.y == board_size) {
        oxocard.matrix->clearScreen();
        oxocard.matrix->drawCharBigFont('V', 2, 0, GREEN);
        delay(2000);
        break;
      }

      player_position = move_player(player_position, player_direction);

      last_millis = current_millis;
    }
  }

  Serial.println("exit");
}
