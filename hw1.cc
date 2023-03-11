#include <stdio.h>
#include <stdlib.h>
// #include <string.h>

#include <iostream>
#include <fstream>
#include <string>
#include <bitset>

#include <list>
#include <queue>
#include <unordered_map>

#include <signal.h>

using namespace std;




unsigned int probe_vstdCount;
unsigned int probe_vstdClidCount;
int probe_nextStateQueue_max_size;

void probe_get_nextStateQueue_max_size(int queueSize){
    if(queueSize > probe_nextStateQueue_max_size){
        probe_nextStateQueue_max_size = queueSize;
    }
}

void probe_print_stat(){
    fprintf(stderr, "\n\n");

    fprintf(stderr, "probe_vstdCount: %d\n", probe_vstdCount);
    fprintf(stderr, "probe_vstdClidCount: %d\n", probe_vstdClidCount);
    fprintf(stderr, "probe_nextStateQueue_max_size: %d\n", probe_nextStateQueue_max_size);
    
    fprintf(stderr, "\n\n");
}




struct State{
    bitset<64> boxPos;
    unsigned char row, col;
};

struct Pos{
    unsigned char row, col;
};

typedef unsigned short poskey_t;

unsigned short pos2key(Pos pos){    
    return ((((unsigned short)pos.row) << 8) | ((unsigned short)pos.col));
}


enum Dir{
    UP,
    RIGHT,
    DOWN,
    LEFT
};

struct Action{
    unsigned char row, col;
    Dir dir;
};

struct PosNode{
    Pos pos;
    PosNode *next;
};


void print_action(Action action){
    fprintf(stderr, "row: %d, col: %d, dir: %d\n", action.row, action.col, (int)action.dir);
}


void print_action_list(list<Action> action_list){
    
    if(action_list.empty()){
        fprintf(stderr, "action_list empty\n");
        return;
    }

    fprintf(stderr, "\n");

    Action action;
    while(!action_list.empty()){

        action = action_list.front();
        action_list.pop_front();

        print_action(action);

    }

    fprintf(stderr, "\n");
}






class Map{
    public:

        Map(char* filename){
            
            read_map(filename);
        }


        void print_map(char* map){
            

            for (int i=0; i<fileLen; i++){
                fprintf(stderr, "%c", map[i]);
            }

            fprintf(stderr, "\n\n");
        }    

        void print_state(State state){

            char* renderedMap = new char[fileLen];
            render_boxPos(renderedMap, state.boxPos);

            Pos pos;
            char mapObj;

            pos.row = state.row;
            pos.col = state.col;

            mapObj = get_map_object(renderedMap, pos);
            if(mapObj == '.') set_map_object(renderedMap, pos, 'O');
            else if(mapObj == ' ') set_map_object(renderedMap, pos, 'o');

            for (int i=0; i<fileLen; i++){
                fprintf(stderr, "%c", renderedMap[i]);
            }
            fprintf(stderr, "\n\n");

        }



        void _get_file_info(char* filename){

            FILE* ptr;
            char ch;
        
            ptr = fopen(filename, "r");
            if (NULL == ptr) {
                fprintf(stderr, "file can't be opened \n");
                exit(-1);
            }            

            fileLineNum = 0;
            fileLen = 0;           
            spaceNum = 0; 
            ch = fgetc(ptr);
            while (ch != EOF){  
                fileLen++;
                if(ch == '\n') fileLineNum++;
                else if(ch != '\n' && ch != EOF && ch != '#') spaceNum++;
                fprintf(stderr, "%c", ch);
                ch = fgetc(ptr);
            }

            fprintf(stderr, "\n");
            fprintf(stderr, "fileLen: %d\n", fileLen);
            fprintf(stderr, "fileLineNum: %d\n", fileLineNum);
            fprintf(stderr, "spaceNum: %d\n", spaceNum);

            fclose(ptr);

        }


        void _read_map(char* filename){
            FILE* ptr;
            char ch;

            map = new char[fileLen];
            // renderedMap = new char[fileLen];
            int offset = 0;

 
            ptr = fopen(filename, "r");
            if (NULL == ptr) {
                fprintf(stderr, "file can't be opened \n");
                exit(-1);
            }
            
            for(int i=0; i<fileLen;i++){
                ch = fgetc(ptr); 
                if (ch == '#' || ch == '\n' || ch == EOF || ch == ' '){
                    map[offset++] = ch;
                }
                else if(ch == 'X' || ch == 'O' || ch == '.'){
                    map[offset++] = '.';
                }
                else{
                    map[offset++] = ' ';
                }  
            }      

            fclose(ptr);
        }


        void _read_state(char* filename){
            FILE* ptr;
            char ch;


            if(spaceNum > 64){
                fprintf(stderr, "spaceNum > 64 \n");
                exit(-1); 
            }
 
            ptr = fopen(filename, "r");
            if (NULL == ptr) {
                fprintf(stderr, "file can't be opened \n");
                exit(-1);
            }

            state.boxPos = 0b0;
            int offset = 0;
            int row = 0, col = 0;

            for(int i=0; i<fileLen-1;i++){
                
                ch = fgetc(ptr); 

                if(ch == '\n'){
                    row++; col = 0;
                    continue;
                }
                
                if(ch != '#'){
                    if(ch == ' ' || ch == '.'){
                        state.boxPos.reset(size_t(offset++));
                    }
                    else if(ch == 'x' || ch == 'X'){
                        state.boxPos.set(size_t(offset++), true);
                    }
                    else if(ch == 'o' || ch == 'O'){
                        state.boxPos.reset(size_t(offset++));
                        state.row = row;
                        state.col = col;
                    }                    
                }

                col++;
            }      

            fclose(ptr);
        }

        void _get_row_ends(){

            rowBegins = new int[fileLineNum];


            int offset = 0;
            rowBegins[offset++] = 0;

            for (int i=1; i<fileLen; i++){
                if(map[i] == '\n'){
                    rowBegins[offset++] = i+1;
                }                
            }  

        }

        void read_map(char* filename){

            _get_file_info(filename);
            _read_map(filename);
            _read_state(filename);
            _get_row_ends();
        }

        bool is_done(State state){
            char* renderedMap = new char[fileLen];
            render_boxPos(renderedMap, state.boxPos);
            char ch;

            for (int i=0; i<fileLen-1; i++){
                ch = renderedMap[i];
                if(ch == 'x') return false;
                if(ch == '.') return false;
                if(ch == 'O') return false;
            }
            return true;
            
        }

        

        void get_available_actions(State state, list<Action>* action_list){
            // Do BFS to find what `action`'s are available given `state`.
    
            char* renderedMap = new char[fileLen];
            render_boxPos(renderedMap, state.boxPos);

            // fprintf(stderr, "\n[get_available_actions()] initial map to find available action:\n\n"); 
            // print_state(state);

            

            queue<Pos> nextPosQueue;
            unordered_map<poskey_t, bool> vstdPosTbl;

            Pos curPos{.row{state.row}, .col{state.col}};
            Pos nextPos;
            Action action;
            nextPosQueue.push(curPos);

            while(!nextPosQueue.empty()){
                

                curPos = nextPosQueue.front();
                nextPosQueue.pop();
                vstdPosTbl[pos2key(curPos)] = true;



                add_local_action(renderedMap, curPos, action_list); // ok

                // fprintf(stderr, "\n[get_available_actions()] print local action:\n\n"); 
                // print_action_list(*action_list);
                


                for(int dir=0; dir<=3; dir++){
                    add_unvisited_nextPos(renderedMap, curPos, (Dir)dir, &vstdPosTbl, &nextPosQueue);
                }
            }
        }



        void add_local_action(char* renderedMap, Pos curPos, list<Action>* action_list){
            // - Find out what adjacent boxes can be pushed.
            // - Do nothing if no box is adjacent.

            Pos probe_pos;
            char mapObj;
            Action action;

            for(int dir=0; dir <=3; dir++){
                
                move(curPos, (Dir)dir, &probe_pos);
                mapObj = get_map_object(renderedMap, probe_pos); 

                // fprintf(stderr, "\n[add_local_action()] mapObj: %c\n\n", mapObj);

                if(mapObj == 'x' || mapObj == 'X'){
                
                    move(probe_pos, (Dir)dir, &probe_pos);
                    mapObj = get_map_object(renderedMap, probe_pos);      

                    if(mapObj == ' ' || mapObj == '.'){
                        action.dir = (Dir)dir;
                        action.row = curPos.row;
                        action.col = curPos.col;

                        if(!is_dead_action(renderedMap, action)){

                            // fprintf(stderr, "\n[add_local_action()] is not dead action.\n\n"); 
                            action_list->push_back(action);

                        }
                        else{
                            // fprintf(stderr, "\n[add_local_action()] is dead action. dir: %d\n\n", (int)dir); 
                        }
                    }
                }
            }
        }



        bool is_dead_action(char* renderedMap, Action action){

            Pos curPos{.row{action.row}, .col{action.col}};
            Pos o_pos, x_pos; // next pos
            
            move(curPos, action.dir, &o_pos);
            move(o_pos, action.dir, &x_pos);

            char* nextRenderedMap = new char[fileLen];
            copy_map(renderedMap, nextRenderedMap);



            // fprintf(stderr, "\n[is_dead_action()] after render boxPos:\n\n");
            // print_map(nextRenderedMap);    

            // fprintf(stderr, "\n[is_dead_action()] print action:\n");
            // print_action(action);


            safe_place_object(nextRenderedMap, o_pos, ' '); // remove 'x'.


            // fprintf(stderr, "\n[is_dead_action()] after delete the x to be moved:\n\n");
            // print_map(nextRenderedMap); 



            if(is_dead_corner(nextRenderedMap, x_pos)){
                // fprintf(stderr, "\n[is_dead_action()] is_dead_corner.\n\n");
                return true;
            }
            // else if(is_pushPos_unreachable(o_pos, x_pos)){
            //     fprintf(stderr, "\n[is_dead_action()] is_pushPos_unreachable.\n\n");
            //     return true;
            // }  

            return false;         
        }



        void copy_map(char* fromMap, char* toMap){
            for(int i=0; i<fileLen; i++){
                toMap[i] = fromMap[i];
            }
        }


        bool is_dead_corner(char* renderedMap, Pos x_pos){
            // Fast path
            // - a box pushed into corner (2 '#' or 1 '#' + 1 'x') and is not on '.' 

            int dir1, dir2;
            Pos probe_pos, probe_pos1, probe_pos2;
            char mapObj, mapObj_adj1, mapObj_adj2, mapObj_x_pos;
            bool is_dead;

            mapObj_x_pos = get_map_object(renderedMap, x_pos);

            for(int dir=0; dir<=3; dir++){

                // fprintf(stderr, "\n[is_dead_corner()] dir: %d.\n\n", dir);



                dir1 = dir; 
                dir2 = (dir1 + 1)%4;

                move(x_pos, (Dir)dir1, &probe_pos1);
                mapObj_adj1 = get_map_object(renderedMap, probe_pos1);

                move(x_pos, (Dir)dir2, &probe_pos2);
                mapObj_adj2 = get_map_object(renderedMap, probe_pos2);


                if(mapObj_adj1 == '#' && mapObj_adj2 == '#' && mapObj_x_pos != '.'){ 
                    // fprintf(stderr, "\n[is_dead_corner()] type 0\n\n");
                    return true;
                }
                else if(mapObj_adj1 == '#' && mapObj_adj2 == 'x' ||
                        mapObj_adj1 == '#' && mapObj_adj2 == 'X' && mapObj_x_pos == ' '){
                    
                    is_dead = true;

                    move(probe_pos2, (Dir)((dir2 + 1)%4), &probe_pos);
                    mapObj = get_map_object(renderedMap, probe_pos);
                    is_dead = is_dead && (mapObj != ' ' && mapObj != '.');

                    move(probe_pos2, (Dir)((dir2 + 3)%4), &probe_pos);
                    mapObj = get_map_object(renderedMap, probe_pos);
                    is_dead = is_dead && (mapObj != ' ' && mapObj != '.');

                    if(is_dead){
                        // fprintf(stderr, "\n[is_dead_corner()] type 1\n\n");
                        return true;
                    } 

                }                   
                else if(mapObj_adj1 == 'x' && mapObj_adj2 == '#' ||
                        mapObj_adj1 == 'X' && mapObj_adj2 == '#' && mapObj_x_pos == ' '){
                    
                    is_dead = true;

                    move(probe_pos1, (Dir)((dir1 + 1)%4), &probe_pos);
                    mapObj = get_map_object(renderedMap, probe_pos);
                    is_dead = is_dead && (mapObj != ' ' && mapObj != '.');

                    move(probe_pos1, (Dir)((dir1 + 3)%4), &probe_pos);
                    mapObj = get_map_object(renderedMap, probe_pos);
                    is_dead = is_dead && (mapObj != ' ' && mapObj != '.');

                    if(is_dead){
                        // fprintf(stderr, "\n[is_dead_corner()] type 2\n\n");
                        return true;
                    } 
                }
                else if((mapObj_adj1 == 'x' && mapObj_adj2 == 'x') ||
                        (mapObj_adj1 == 'x' && mapObj_adj2 == 'X') ||
                        (mapObj_adj1 == 'X' && mapObj_adj2 == 'x') ||
                        (mapObj_adj1 == 'X' && mapObj_adj2 == 'X' && mapObj_x_pos == ' ')){
                    
                    is_dead = true;

                    move(probe_pos1, (Dir)((dir1 + 1)%4), &probe_pos);
                    mapObj = get_map_object(renderedMap, probe_pos);
                    is_dead = is_dead && (mapObj != ' ' && mapObj != '.');

                    move(probe_pos1, (Dir)((dir1 + 3)%4), &probe_pos);
                    mapObj = get_map_object(renderedMap, probe_pos);
                    is_dead = is_dead && (mapObj != ' ' && mapObj != '.');
                    

                    move(probe_pos2, (Dir)((dir2 + 1)%4), &probe_pos);
                    mapObj = get_map_object(renderedMap, probe_pos);
                    is_dead = is_dead && (mapObj != ' ' && mapObj != '.');

                    move(probe_pos2, (Dir)((dir2 + 3)%4), &probe_pos);
                    mapObj = get_map_object(renderedMap, probe_pos);
                    is_dead = is_dead && (mapObj != ' ' && mapObj != '.');

                    if(is_dead){
                        // fprintf(stderr, "\n[is_dead_corner()] type 3\n\n");
                        return true;
                    }                     
                }              
            }     

            return false;

        }
        

        bool is_pushPos_unreachable(Pos o_pos, Pos x_pos){
            // Slow path
            // 1. find all available action on a box
            // 2. get pos needed to do the action
            // 3. use BFS on `map` to see whether those pos's are reachable.
            // 4. if all of pos are unreachable and 'x' is not on '.', return true.

            char* onexMap = new char[fileLen];
            reset_renderedMap(onexMap);
            safe_place_object(onexMap, x_pos, 'x');
            char mapObj;

            fprintf(stderr, "\n[is_pushPos_unreachable()] safe_place x on onexMap, print_map:\n\n");
            print_map(onexMap);




            fprintf(stderr, "\n[is_pushPos_unreachable()] print debug map:\n\n");
            char* debugMap = new char[fileLen];
            reset_renderedMap(debugMap);
            // copy_map(rawMap, debugMap);
            safe_place_object(debugMap, o_pos, 'o');
            safe_place_object(debugMap, x_pos, 'x');
            print_map(debugMap);

            



            queue<Pos> nextPosQueue;
            queue<Pos> pushPosQueue;
            unordered_map<poskey_t, bool> vstdPosTbl;
            
            Pos pushPos, probe_pos, curPos = o_pos;
            nextPosQueue.push(curPos);
                            
            // find push positions (4 at most).
            for(int dir=0; dir<=3; dir++){
                move(x_pos, (Dir)dir, &probe_pos);
                mapObj = get_map_object(onexMap, probe_pos);
                if(mapObj == ' ' || mapObj == '.') {
                    fprintf(stderr, "\n[is_pushPos_unreachable()] action found, dir: %d\n", dir);
                    pushPosQueue.push(probe_pos);
                }    
            }

            // BFS on onexMap starting from o_pos.
            while(!nextPosQueue.empty()){
                curPos = nextPosQueue.front();
                nextPosQueue.pop();
                vstdPosTbl[pos2key(curPos)] = true;

                for(int dir=0; dir<=3; dir++){
                    add_unvisited_nextPos(onexMap, curPos, (Dir)dir, &vstdPosTbl, &nextPosQueue);
                }
            }
            

            // check whether all push positions cannot be found in vstdPosTbl.
            bool is_all_unreachable = true;
            while(!pushPosQueue.empty()){
                pushPos = pushPosQueue.front();
                pushPosQueue.pop();

                is_all_unreachable = is_all_unreachable && !is_pos_visited(&vstdPosTbl, pushPos);
                if(!is_all_unreachable) break;
            }

            return is_all_unreachable;            
        }



        bool is_pos_visited(unordered_map<poskey_t, bool> *vstdPosTbl, Pos pos){
            return !(vstdPosTbl->find(pos2key(pos)) == vstdPosTbl->end());
        }


        void add_unvisited_nextPos(char *renderedMap, Pos curPos, Dir dir, 
                    unordered_map<poskey_t, bool> *vstdPosTbl, queue<Pos> *nextPosQueue){
            // check whether the position in `dir` is empty space (' ' or '.').
                // If so and if not already explored, add to 'nextPosQueue'.

            // fprintf(stderr, "\n[add_unvisited_nextPos()] dir: %d\n", (int)dir); 

            Pos nextPos;
            char mapObj;

            move(curPos, dir, &nextPos);
            mapObj = get_map_object(renderedMap, nextPos); 
            
            // fprintf(stderr, "[add_unvisited_nextPos()] mapObj: %c\n", mapObj);

            if(mapObj == '.' || mapObj == ' '){
                if(!is_pos_visited(vstdPosTbl, nextPos)){
                    
                    // fprintf(stderr, "[add_unvisited_nextPos()] pos (%d, %d) not visited.\n", nextPos.row, nextPos.col);

                    nextPosQueue->push(nextPos);
                }else{
                    // fprintf(stderr, "[add_unvisited_nextPos()] pos (%d, %d) already visited.\n", nextPos.row, nextPos.col);
                }
            }

            // fprintf(stderr, "\n\n");
        }


        void move(Pos curPos, Dir dir, Pos *nextPos){

            nextPos->row = curPos.row;
            nextPos->col = curPos.col;

            if(dir == UP) nextPos->row -= 1;
            else if(dir == RIGHT) nextPos->col += 1;
            else if(dir == DOWN) nextPos->row += 1;
            else if(dir == LEFT) nextPos->col -= 1;
        }


        State get_state(){
            return state;
        }

        char get_map_object(char *renderedMap, Pos pos){
            return renderedMap[rowBegins[pos.row] + pos.col];
        }

        void render_boxPos(char *renderedMap, bitset<64> boxPos){

            char ch;
            int offset = 0;

            for (int i=0; i<fileLen-1; i++){

                ch = map[i];
                
                if(ch == ' '){
                    if(boxPos[offset++] == 1) ch = 'x';
                }
                else if(ch == '.'){
                    if(boxPos[offset++] == 1) ch = 'X';
                }
                
                renderedMap[i] = ch;
            }
        }

        void reset_renderedMap(char* renderedMap){
            for (int i=0; i<fileLen-1; i++){
                renderedMap[i] = map[i];
            }
        }


        void act(State cur_state, Action action, State* next_state){
            // - Push box, need update position of one 'x' and 'o'.

            char* renderedMap = new char[fileLen];
            render_boxPos(renderedMap, cur_state.boxPos);  

            // fprintf(stderr, "\n[act()]: before action\n\n");
            // print_map(renderedMap);
            // print_state(cur_state);

            char mapObj;
            Pos o_pos, x_pos;
            
            o_pos.row = action.row;
            o_pos.col = action.col;
            move(o_pos, action.dir, &x_pos);

            // replace 'x' in old position by 'o'.
            mapObj = get_map_object(renderedMap, x_pos); 
            if(mapObj == 'x') set_map_object(renderedMap, x_pos, 'o');
            else if(mapObj == 'X') set_map_object(renderedMap, x_pos, 'O');
            else {fprintf(stderr, "[act()] mapObj != 'x' \n"); exit(-1);}

            // place 'x' on its new position. 
            move(x_pos, action.dir, &x_pos);
            mapObj = get_map_object(renderedMap, x_pos); 
            if(mapObj == ' ') set_map_object(renderedMap, x_pos, 'x');
            else if(mapObj == '.') set_map_object(renderedMap, x_pos, 'X');
            else {fprintf(stderr, "[act()] `x` new position not a space. \n"); exit(-1);}

            // fprintf(stderr, "\n[act()]: after action\n\n");
            // print_map(renderedMap);
             

            map2state(renderedMap, next_state);

            // fprintf(stderr, "\n[act()]: after action\n\n");
            // print_state(*next_state);

        }



        void safe_place_object(char* map, Pos pos, char ch){
            char mapObj;
            mapObj = get_map_object(map, pos);

            if(ch == 'o' || ch == 'O'){
                if(mapObj == ' ') set_map_object(map, pos, 'o');
                else if(mapObj == '.') set_map_object(map, pos, 'O');
                else{fprintf(stderr, "\n[safe_place_object()]:invalid placement.\n\n"); exit(-1);}
            }
            else if(ch == 'x' || ch == 'X'){
                if(mapObj == ' ') set_map_object(map, pos, 'x');
                else if(mapObj == '.') set_map_object(map, pos, 'X');
                else{fprintf(stderr, "\n[safe_place_object()]:invalid placement.\n\n"); exit(-1);}
            }
            else if(ch == ' '){
                if(mapObj == 'o' || mapObj == 'x') set_map_object(map, pos, ' ');
                else if(mapObj == 'O' || mapObj == 'X') set_map_object(map, pos, '.');
            }
            else{
                set_map_object(map, pos, ch);
            }        
        }

        void set_map_object(char* map, Pos pos, char ch){
            map[rowBegins[pos.row] + pos.col] = ch;
        }


        void map2state(char* renderedMap, State* state){
            char ch;

            state->boxPos = 0b0;
            int offset = 0;
            int row = 0, col = 0;

            for(int i=0; i<fileLen-1;i++){
                
                ch = renderedMap[i]; 

                if(ch == '\n'){ row++; col = 0; continue; }

                if(ch != '#'){ 

                    if(ch == ' ' || ch == '.'){
                        offset++;
                    }
                    else if(ch == 'x' || ch == 'X'){
                        state->boxPos.set(size_t(offset++), true);
                    }
                    else if(ch == 'o' || ch == 'O'){
                        offset++;
                        state->row = row;
                        state->col = col;
                    }       
                }             

                col++;
            }      
        }


        bool is_reachable(State state, Pos pos1, Pos pos2){
            
            // fprintf(stderr, "\n[is_reachable()]:\n\n"); 

            char* renderedMap = new char[fileLen];
            render_boxPos(renderedMap, state.boxPos);  
            char mapObj;

            queue<Pos> nextPosQueue;
            unordered_map<poskey_t, bool> vstdPosTbl;
            
            Pos probe_pos, curPos = pos1;
            nextPosQueue.push(curPos);
                            
            while(!nextPosQueue.empty()){
                curPos = nextPosQueue.front();
                nextPosQueue.pop();
                vstdPosTbl[pos2key(curPos)] = true;

                for(int dir=0; dir<=3; dir++){
                    add_unvisited_nextPos(renderedMap, curPos, (Dir)dir, &vstdPosTbl, &nextPosQueue);
                }
            }

            if(is_pos_visited(&vstdPosTbl, pos2)){

                // if((pos1.row != pos2.row) && (pos1.col != pos2.col)){
                //     State tmpSt;
                //     tmpSt.boxPos = state.boxPos;

                //     fprintf(stderr, "\n[is_reachable()] is visited. print pos1 state:\n\n");
                //     tmpSt.row = pos1.row;
                //     tmpSt.col = pos1.col;
                //     print_state(tmpSt);


                //     fprintf(stderr, "\n[is_reachable()] is visited. print pos2 state:\n\n");
                //     tmpSt.row = pos2.row;
                //     tmpSt.col = pos2.col;
                //     print_state(tmpSt);                    
                // }

                return true;
            }
            else{
                
                // if((pos1.row != pos2.row) && (pos1.col != pos2.col)){
                //     State tmpSt;
                //     tmpSt.boxPos = state.boxPos;

                //     fprintf(stderr, "\n[is_reachable()] not visited. print pos1 state:\n\n");
                //     tmpSt.row = pos1.row;
                //     tmpSt.col = pos1.col;
                //     print_state(tmpSt);


                //     fprintf(stderr, "\n[is_reachable()] not visited. print pos2 state:\n\n");
                //     tmpSt.row = pos2.row;
                //     tmpSt.col = pos2.col;
                //     print_state(tmpSt);             
                // }



                return false;
            }
        }



        char *map;
        // char *renderedMap;

        int *rowEnds;
        int *rowBegins;

        State state;        
   
        int fileLen;
        int fileLineNum;
        int spaceNum;
};







class Solver{
    public:

    Solver(char* filename){
        map = new Map(filename);

        map->print_map(map->map);
        explore();
    }


    void explore(){
        
        State state, nextState;
        list<Action> action_list;

        nextStateQueue.push(map->get_state());
        
        bool done = false;



        while (!done && !nextStateQueue.empty()) {
            probe_vstdCount++;
            probe_get_nextStateQueue_max_size(nextStateQueue.size());


            // Wrong!! only check with those state in `vstdStTbl` or `vstdClidStTbl`,
                // but fail to check with those state in `nextStateQueue`.


            state = nextStateQueue.front();
            nextStateQueue.pop();

            // add_visited_state(&vstdStTbl, &vstdClidStTbl, state);    

            map->get_available_actions(state, &action_list); 


            // fprintf(stderr, "\n[explore()] return from get_available_actions(). print action list:\n\n");
            // print_action_list(action_list);

            while(!action_list.empty()){
             
                map->act(state, action_list.front(), &nextState);
                action_list.pop_front();
                
                if(map->is_done(nextState)){
                    fprintf(stderr, "\n[explore()]: done!\n\n"); 
                    map->print_state(nextState);
                    done = true; 

                    fprintf(stderr, "\n\n[explore()]: Print stat:");
                    probe_print_stat();
                    break;
                } 



                // Wrong!! only check with those state in `vstdStTbl` or `vstdClidStTbl`,
                    // but fail to check with those state in `nextStateQueue`.



                if(!is_state_visited(&vstdStTbl, &vstdClidStTbl, nextState)){ 
                    // fprintf(stderr, "\n[explore()] nextState not visited, will be add to queue:\n\n");
                    // map->print_state(nextState);
                    add_visited_state(&vstdStTbl, &vstdClidStTbl, nextState);    
                    nextStateQueue.push(nextState);
                }    
            }
        }
    }





    void add_visited_state(unordered_map<bitset<64>, Pos> *vstdStTbl, 
                    unordered_map<bitset<64>, PosNode*> *vstdClidStTbl, State state){
        
        Pos pos;
        pos.row = state.row;
        pos.col = state.col;

        if(!is_in_vstdStTbl(vstdStTbl, state.boxPos)){ // not inside.
            (*vstdStTbl)[state.boxPos] = pos;
        }else{


            fprintf(stderr, "\n[add_visited_state()] print new state:\n\n");
            map->print_state(state);

            fprintf(stderr, "\n[add_visited_state()] print first visited state:\n\n");

            Pos vstdPos = (*vstdStTbl)[state.boxPos];
            State vstdState;
            vstdState.row = vstdPos.row;
            vstdState.col = vstdPos.col;
            vstdState.boxPos = state.boxPos;
            
            map->print_state(vstdState);



            probe_vstdClidCount++;
            insert_PosNode(vstdClidStTbl, state);
        }
    }


    void insert_PosNode(unordered_map<bitset<64>, PosNode*> *vstdClidStTbl, State state){
        
        Pos pos;
        pos.row = state.row;
        pos.col = state.col;

        PosNode *cur = new PosNode;
        cur->pos = pos;

        if(!is_in_vstdClidStTbl(vstdClidStTbl, state.boxPos)){
            cur->next = NULL;
        }
        else{
            cur->next = (*vstdClidStTbl)[state.boxPos];
        }

        (*vstdClidStTbl)[state.boxPos] = cur;
    }


    bool is_state_visited(unordered_map<bitset<64>, Pos> *vstdStTbl, 
                    unordered_map<bitset<64>, PosNode*> *vstdClidStTbl, State state){



        // key not found -> haven't been visited.
        if(!is_in_vstdStTbl(vstdStTbl, state.boxPos)){return false;}


        Pos visitedPos = (*vstdStTbl)[state.boxPos];
        Pos curPos{.row{state.row}, .col{state.col}};


        // Have same box positions, and mutually 
            // reachable player positions *with the head* -> deemed visited.
        if(map->is_reachable(state, curPos, visitedPos)){return true;}
        

        // key not found -> haven't been visited.
        if(!is_in_vstdClidStTbl(vstdClidStTbl, state.boxPos)){return false;}


  
        PosNode *cur_ptr = (*vstdClidStTbl)[state.boxPos];

        while(cur_ptr){

            visitedPos = cur_ptr->pos;

            // Have same box positions, and mutually 
                // reachable player positions *with other collided pos*. 
            if(map->is_reachable(state, curPos, visitedPos)){return true;}
            
            cur_ptr = cur_ptr->next;
        }

        return false;
    }
    

    bool is_in_vstdClidStTbl(unordered_map<bitset<64>, PosNode*> *vstdClidStTbl, 
                                                                            bitset<64> boxPos){
        return !(vstdClidStTbl->find(boxPos) == vstdClidStTbl->end());
    }


    bool is_in_vstdStTbl(unordered_map<bitset<64>, Pos> *vstdStTbl, bitset<64> boxPos){
        return !(vstdStTbl->find(boxPos) == vstdStTbl->end());
    }


    unordered_map<bitset<64>, Pos> vstdStTbl;
    unordered_map<bitset<64>, PosNode*> vstdClidStTbl;
    queue<State> nextStateQueue;
    

    Map* map;
};




void signal_callback_handler(int signum) {
    
    fprintf(stderr, "\n\nTerminate program. Print stat:");

    probe_print_stat();

    exit(signum);
}


int main(int argc, char** argv) {

    signal(SIGINT, signal_callback_handler);

    fprintf(stderr, "argv[1]: %s\n", argv[1]);

    probe_vstdCount = 0;
    probe_vstdClidCount = 0;
    probe_nextStateQueue_max_size = 0;

    Solver solver(argv[1]);
    

    return 0;
}