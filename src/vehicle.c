#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

#include "../include/vehicle.h"
#include "../include/map.h"
#include "../include/clock.h"
#include "../include/sync.h"

#define VEHICLE_LIFETIME_TICKS 149

static bool should_move(Vehicle *vehicle, int tick){
    if (!vehicle) return false;

    switch (vehicle->speed){
        case SPEED_FAST:   return true;
        case SPEED_MEDIUM: return (tick % 2 == 0);
        case SPEED_SLOW:   return (tick % 4 == 0);
        default:           return false; 
    }
}

static Direction opposite_direction(Direction dir){
    switch (dir){
        case DIR_NORTH: return DIR_SOUTH;
        case DIR_SOUTH: return DIR_NORTH;
        case DIR_EAST:  return DIR_WEST;
        case DIR_WEST:  return DIR_EAST;
        default:        return DIR_INVALID;
    }
}

typedef enum { TURN_RIGHT, TURN_LEFT } TurnSide;

static Direction turn_direction(Direction dir, TurnSide side){
    if (side == TURN_RIGHT){
        switch (dir){
            case DIR_NORTH: return DIR_EAST;
            case DIR_SOUTH: return DIR_WEST;
            case DIR_WEST:  return DIR_NORTH;
            case DIR_EAST:  return DIR_SOUTH;
            default:        return DIR_INVALID;
        }
    }
    switch (dir){
        case DIR_NORTH: return DIR_WEST;
        case DIR_SOUTH: return DIR_EAST;
        case DIR_WEST:  return DIR_SOUTH;
        case DIR_EAST:  return DIR_NORTH;
        default:        return DIR_INVALID;
    }
}

static Pos pos_from_direction(Pos from, Direction dir){
    Pos next = from;
    switch (dir){
        case DIR_NORTH: next.row--; break;
        case DIR_SOUTH: next.row++; break;
        case DIR_EAST:  next.col++; break; 
        case DIR_WEST:  next.col--; break; 
        default: break;
    }
    return next;
}

static char vehicle_symbol(const Vehicle *vehicle){
    if (vehicle->type == TYPE_AMBULANCE) return 'A';
    return 'C';
}

static bool move_to(Vehicle *vehicle, Pos next){
    Pos old_pos = vehicle->pos;

    if (!cell_try_occupy(vehicle->map, next.row, next.col, vehicle->id)){
        return false; 
    }

    vehicle->map->grid[next.row][next.col].occupant_symbol = vehicle_symbol(vehicle);
    vehicle->pos = next;
    cell_release(vehicle->map, old_pos.row, old_pos.col);
    return true;
}

static Direction pick_intersection_exit(Vehicle *vehicle){
    Direction straight = vehicle->direction;
    Direction right    = turn_direction(straight, TURN_RIGHT);
    Direction left     = turn_direction(straight, TURN_LEFT);
    Direction back     = opposite_direction(straight);

    Direction candidates[4];
    int roll = rand_r(&vehicle->rng_state) % 3;
    if (roll == 0){
        candidates[0] = straight; candidates[1] = right; candidates[2] = left;
    } else if (roll == 1){
        candidates[0] = right; candidates[1] = straight; candidates[2] = left;
    } else {
        candidates[0] = left; candidates[1] = straight; candidates[2] = right;
    }
    candidates[3] = back;

    for (int i = 0; i < 4; i++){
        Direction dir = candidates[i];
        Pos candidate_next = pos_from_direction(vehicle->pos, dir);
        Direction check = map_is_valid_move(vehicle->map, candidate_next.row, candidate_next.col, dir);
        if (check != DIR_INVALID){
            return dir;
        }
    }

    return DIR_INVALID;
}

static bool try_enter_intersection(Vehicle *vehicle, Pos next){
    if (vehicle->type == TYPE_AMBULANCE){
        traffic_request_priority(next.row, next.col, vehicle->direction);
    }

    traffic_wait_green(next.row, next.col, vehicle->direction);

    return move_to(vehicle, next);
}

static bool advance_out_of_intersection(Vehicle *vehicle){
    Direction exit_dir = pick_intersection_exit(vehicle);
    if (exit_dir == DIR_INVALID){
        return false;
    }

    vehicle->direction = exit_dir;
    Pos next = pos_from_direction(vehicle->pos, exit_dir);
    return move_to(vehicle, next);
}

static bool car_advance(Vehicle *vehicle, int tick){
    if (!should_move(vehicle, tick)) return false;

    CellType current_type = vehicle->map->grid[vehicle->pos.row][vehicle->pos.col].type;

    if (current_type == CELL_INTERSECTION){
        return advance_out_of_intersection(vehicle);
    }

    Pos next = pos_from_direction(vehicle->pos, vehicle->direction);
    Direction check = map_is_valid_move(vehicle->map, next.row, next.col, vehicle->direction);

    if (check == DIR_INVALID){
        vehicle->direction = opposite_direction(vehicle->direction);
        return true;
    }

    CellType next_type = vehicle->map->grid[next.row][next.col].type;

    if (next_type == CELL_INTERSECTION){
        return try_enter_intersection(vehicle, next);
    }

    if (check == 0){
        vehicle->direction = opposite_direction(vehicle->direction);
        return true;
    }

    return move_to(vehicle, next);
}

Vehicle *vehicle_init(Vehicle *vehicle, int id, VehicleType t, Direction d, Pos start, Speed s, Map *m){
    vehicle->id = id;
    vehicle->type = t;
    vehicle->direction = d;
    vehicle->pos = start;
    vehicle->speed = s;
    vehicle->map = m;

    vehicle->rng_state = (unsigned int)time(NULL) ^ ((unsigned int)id * 2654435761u);

    return vehicle;
}

void vehicle_run(Vehicle *vehicle){
    int global_tick = 0;
    int move_tick = 0;
    extern int is_active;

    while (global_tick < VEHICLE_LIFETIME_TICKS && is_active){
        global_tick = clock_get_tick();
        move_tick++;
        car_advance(vehicle, move_tick);
        clock_wait_tick();
    }
}
