#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>

#include "../include/vehicle.h"
#include "../include/map.h"
#include "../include/clock.h"
#include "../include/sync.h"

struct Pos{
    int row;
    int col;
};

struct Vehicle{
    int id;
    VehicleType type;
    Direction direction;
    Speed speed;
    Pos pos;
    Map *map;
};

static bool should_move(Vehicle *vehicle){
    if (!vehicle) return false;

    if (map_is_valid_move){
        int tick = clock_get_tick();
        switch (vehicle->speed){
            case SPEED_FAST: return true;
            case SPEED_MEDIUM: return (tick % 2 == 0);
            case SPEED_SLOW: return (tick % 4 == 0);
            default: false;
        }
    }
}

static Direction dir_from_tile(Vehicle *vehicle){
    int x = vehicle->pos.col;
    int y = vehicle->pos.row;
    CellType tile_type = vehicle->map->grid[y][x].type;
    switch (tile_type) {
        case CELL_ONE_WAY_N: return DIR_NORTH;
        case CELL_ONE_WAY_S: return DIR_SOUTH;
        case CELL_ONE_WAY_E: return DIR_EAST;
        case CELL_ONE_WAY_W: return DIR_WEST;
        case CELL_INTERSECTION: return DIR_COUNT;
        default: return DIR_COUNT;
    }
}

static Pos car_next_pos(Vehicle *vehicle){
    Pos next = vehicle->pos;
    Direction dir = vehicle->direction;

    switch (dir){
        DIR_NORTH: next.row--; break;
        DIR_SOUTH: next.row++; break;
        DIR_EAST: next.col--; break;
        DIR_WEST: next.col++; break;
        default: break;
    }
    return next;
}

static void car_advance(Vehicle *vehicle, Pos next){
    Direction check = map_is_valid_move(vehicle->map, next.row, next.col, vehicle->direction);

    if (check != 0){
        switch (check){
            //case DIR_INVALID: break; /* for better case handling

            case DIR_COUNT: break; // *signal/intersection treatment
            
            default:
            vehicle->pos = next;
        }
    }
    return false;
}

static void car_wait_green(Vehicle *vehicle, Pos signal){
}

Vehicle *vehicle_init(Vehicle *vehicle, int id, VehicleType t, Direction d, Pos start, Speed s, Map *m){
    vehicle->id = id;
    vehicle->type = t;
    vehicle->direction = d;
    vehicle->pos = start;
    vehicle->speed = s;
    vehicle->map = m;
    return vehicle;
}

void vehicle_run(Vehicle *vehicle){
    Map *map = vehicle->map;
    while (1/*change to ticks*/){
        Pos pos = vehicle->pos;
        Direction tile_dir = dir_from_tile(vehicle);
        switch (tile_dir){
            case DIR_COUNT: continue;
            default: Pos next = car_next_pos(vehicle);
            car_advance(vehicle, next);
            }
        }
}