from dataclasses import dataclass

import requests
import sys
import os

from processlib import get_lines, get_lines_csv
from process_entity_info import process_entity_info


@dataclass
class State:
    stateID: int
    state: str
    nextState: str
    numFrames: int
    spriteState: str


def process_states():
    print("Process entity states...")
    entity_infos = process_entity_info()

    def write_out(s):
        myfile.write(s)

    lines = get_lines_csv('dev/entity_states.csv')

    state_idx = 0

    myfile = open("src/res/res.pas", "w")

    states = []
    sprite_states = {}

    stateID = 0
    spriteStateID = 0
    for line in lines:
        sprite_state = f'SPRITE_STATE_{line[3]}'
        if sprite_state not in sprite_states:
            sprite_states[sprite_state] = spriteStateID
            spriteStateID += 1

        states.append(State(stateID=stateID, state=f'STATE_{line[0]}', nextState=f'STATE_{line[1]}', numFrames=int(line[2]), spriteState=sprite_state))
        stateID += 1

    write_out("""
unit res;
interface
uses common;

const
""")

    for state in states:
        write_out(f"\t{state.state} = {state.stateID};\n")

    write_out(f"\tSTATE_MAX = {stateID};\n")

    write_out("\n")
    for sprite_state in sprite_states:
        write_out(f"\t{sprite_state} = {sprite_states[sprite_state]};\n")

    write_out(f"\tSPRITE_STATE_MAX = {spriteStateID};\n\n")

    # Sprite state data?

    for et in entity_infos:
        write_out(f"\t{et.entityTypeName} = {et.classID};\n")

    write_out("""
	entity_states: array[0..STATE_MAX-1] of TEntityState = (
""")

    for state in states:
        write_out(f"\t\t(state: {state.state}; nextState: {state.nextState}; numFrames: {state.numFrames}; spriteState: {state.spriteState})")
        if state != states[-1]:
            write_out(',')
        write_out('\n')

    write_out("\t);\n")

    num_entity_infos = len(entity_infos)
    write_out(f"""
    var
        ent_info: array[0..{num_entity_infos - 1}] of TEntityInfo;\n""")

    write_out("implementation\n")
    write_out("begin\n")

    # for i in entity_infos:
    #     write_out(f'\t\t(mins: (x:{i.mins[0]}; y:{i.mins[1]});\n')
    #     write_out(f'\t\t\tmaxs: (x:{i.maxs[0]}; y:{i.maxs[1]});\n')
    #     write_out(f'\t\t\tcollision: {i.collision};\n')
    #     write_out(f'\t\t\tentityFrameFunc: {i.frame_func};\n')
    #     write_out(f'\t\t\tstateChangeFunc: {i.state_change_func}\n')
    #     write_out('\t\t)')
    #
    #     if i != entity_infos[-1]:
    #         write_out(',')
    #     write_out('\n')
    #
    # write_out("\t);\n")
    write_out("end.\n")
    myfile.close()