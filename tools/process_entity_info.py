from dataclasses import dataclass
from typing import Tuple, Optional

import requests
import sys
import os

from processlib import get_lines_csv
from processlib import get_lines_csv_as_dict


@dataclass
class EntityInfo:
    classID: int
    entityTypeName: str
    mins: Tuple[int, int]
    maxs: Tuple[int, int]
    # origin: Tuple[int, int]
    collision: int
    damage_func: Optional[str]
    frame_func: Optional[str]
    hit_func: Optional[str]
    hitby_func: Optional[str]
    state_change_func: Optional[str]


def process_entity_info() -> list[EntityInfo]:
    print("Process entity info...")


    infos = []

    # sys.stdout.write(s)

    lines = get_lines_csv_as_dict('dev/entity_info.csv')

    state_idx = 0
    lines2 = []

    # for line in lines:
    # 	write_header("#define ENTITY_{} {}\n".format(line[0], state_idx))
    # 	state_idx += 1

    # write_header("extern const entity_state_t entity_states[{}];\n".format(len(lines2)))
    # write_header("extern const ent_info_t ent_info[];\n")
    # write_out("const ent_info_t ent_info[{}] = {{\n".format(len(lines)))

    classID = 0
    for line in lines:
        width = int(line['width'])
        height = int(line['height'])
        offset_x = int(line['origin_x'])
        offset_y = int(line['origin_y'])

        mins = (-offset_x, -offset_y)
        maxs = (width - offset_x, height - offset_y)


        infos.append(EntityInfo(
            classID=classID,
            entityTypeName=f"ET_{line['type']}",
            mins=mins,
            maxs=maxs,
            collision=int(line['collision']),
            damage_func=line['damage_func'] or "nil",
            frame_func=line['frame_func'] or "nil",
            hit_func=line['hit_func'] or "nil",
            hitby_func=line['hitby_func'] or "nil",
            state_change_func=line['state_change_func'] or "nil",
        ))

        classID += 1

    # print(infos)
    return infos