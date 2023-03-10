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

// using namespace std;



struct State{
    bitset<64> boxPos;
    unsigned char row, col;
};

struct Pos{
    unsigned char row, col;
};

enum Dir{
    up,
    right,
    down,
    left
};

struct Action{
    unsigned char row, col;
    Dir dir;
};




class Map{
    public:

        Map(char* filename){
            
            read_map(filename);
        }

        void print_map(){
            
            fprintf(stderr, "\n[print_map()]:\n\n");

            for (int i=0; i<fileLen; i++){
                fprintf(stderr, "%c", map[i]);
            }
        }    


        void print_map_state(){
            
            fprintf(stderr, "\n[print_map_state()]:\n\n");

            char ch;
            int offset = 0;
            int row = 0, col = 0;

            for (int i=0; i<fileLen-1; i++){
                ch = map[i];
                
                if(ch == '\n'){
                    row++; col = 0;
                    // continue;
                }
                else{

                
                    if(ch == ' '){
                        if(state.boxPos[offset++] == 1){
                            ch = 'x';
                        }
                        else if(row == state.row && col == state.col){
                            ch = 'o';
                        }
                    }
                    else if(ch == '.'){
                        if(state.boxPos[offset++] == 1){
                            ch = 'X';
                        } 
                        else if(row == state.row && col == state.col){
                            ch = 'O';
                        }                    
                    }

                    col++;
                
                }

                fprintf(stderr, "%c", ch);

            }




            // fprintf(stderr, "\n\n[test get_map_object()]:\n\n");

            // Pos pos;
            // char obj;

            // for(int i =0; i<9; i++){
            //     for(int j =0; j<10; j++){
            //         pos.row = i; pos.col = j;
            //         fprintf(stderr, "%c", get_map_object(pos));
            //     }
            // }
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
                else if(ch == 'X' || ch == '.'){
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

        }



        void get_available_actions(State state, list<Action>* action_list){
            // Do BFS to find what `action`'s are available given `state`.

            char* renderedMap = new char[fileLen];
            render_boxPos(renderedMap, state.boxPos);

            queue<Pos> nextPosQueue;
            unordered_map<Pos, bool> visitedPos;

            Pos curPos{.row{state.row}, .col{state.col}};
            Pos nextPos;
            Action action;
            nextPosQueue.push(curPos);

            while(!nextPosQueue.empty()){
                curPos = nextPosQueue.front();
                nextPosQueue.pop();
                visitedPos[curPos] = true;

                add_local_action(renderedMap, curPos, &action_list);
                
                for(int dir=0; dir<=3; dir++){
                    add_unvisited_nextPos(renderedMap, curPos, (Dir)dir, &visitedPos, &nextPosQueue);
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

                if(mapObj == 'x' || mapObj == 'X'){
                
                    move(probe_pos, (Dir)dir, &probe_pos);
                    mapObj = get_map_object(renderedMap, probe_pos);      

                    if(mapObj == ' ' || mapObj == '.'){
                        action.dir = (Dir)dir;
                        action.row = curPos.row;
                        action.col = curPos.col;
                        if(!willbe_dead_state(renderedMap, action)) action_list->push_back(action);
                    }
                }
            }
        }

        bool willbe_dead_state(char* renderedMap, Action action){

            Pos curPos{.row{action.row}, .col{action.col}};
            Pos o_pos, x_pos;
            
            move(curPos, action.dir, &o_pos);
            move(o_pos, action.dir, &x_pos);

            if(is_dead_corner(renderedMap, x_pos)){return true;}
            else if(is_pushPos_unreachable(renderedMap, x_pos)){return true;}            
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
                
                dir1 = dir; 
                dir2 = (dir1 + 1)%4;

                move(x_pos, dir1, &probe_pos1);
                mapObj_adj1 = get_map_object(renderedMap, probe_pos1);

                move(x_pos, dir2, &probe_pos2);
                mapObj_adj1 = get_map_object(renderedMap, probe_pos2);


                if(mapObj_adj1 == '#' && mapObj_adj2 == '#' && mapObj_adj != '.'){ 
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

                    if(is_dead) return true;

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

                    return is_dead;

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

                    return is_dead;
                }                    
            }            
        }
        

        void is_pushPos_unreachable(Pos o_pos, Pos x_pos){
            // Slow path
            // 1. find all available action on a box
            // 2. get pos needed to do the action
            // 3. use BFS on `map` to see whether those pos's are reachable.
            // 4. if all of pos are unreachable and 'x' is not on '.', return true.

            char* rawMap = new char[fileLen];
            reset_renderedMap(rawMap);
            char mapObj;

            queue<Pos> nextPosQueue;
            queue<Pos> pushPosQueue;
            unordered_map<Pos, bool> visitedPos;
            
            Pos probe_pos, curPos = o_pos;
            nextPosQueue.push(curPos);
                            
            for(int dir=0; dir<=3; dir++){
                move(x_pos, (Dir)dir, &probe_pos);
                mapObj = get_map_object(rawMap, probe_pos);
                if(mapObj == ' ' || mapObj == '.') pushPosQueue.push(probe_pos);
            }


            while(!nextPosQueue.empty()){
                curPos = nextPosQueue.front();
                nextPosQueue.pop();
                visitedPos[curPos] = true;

                for(int dir=0; dir<=3; dir++){
                    add_unvisited_nextPos(rawMap, curPos, (Dir)dir, &visitedPos, &nextPosQueue);
                }
            }
            
            bool is_all_unreachable = true;
            while(!pushPosQueue.empty()){
                pushPos = pushPosQueue.front();
                pushPosQueue.pop();

                is_all_unreachable = is_all_unreachable && !is_pos_visited(&visitedPos, pushPos);
                if(!is_all_unreachable) break;
            }

            return is_all_unreachable;            
        }





        bool is_pos_visited(unordered_map<Pos, bool> *visitedPos, Pos pos){
            return visitedPos.find(pos) == visitedPos.end();
        }


        bool add_unvisited_nextPos(char *renderedMap, Pos curPos, Dir dir, 
                    unordered_map<Pos, bool> *visitedPos, queue<Pos> *nextPosQueue){
            // check whether the position in `dir` is empty space (' ' or '.').
                // If so and if not already explored, add to 'nextPosQueue'.

            Pos nextPos;
            char mapObj;

            move(curPos, dir, &nextPos);
            mapObj = get_map_object(renderedMap, nextPos); 

            if(mapObj == '.' || mapObj == ' '){
                if(!is_pos_visited(visitedPos, nextPos)){
                    nextPosQueue->push(nextPos);
                }
            }
        }

        void move(Pos curPos, Dir dir, Pos *nextPos){

            nextPos->row = curPos.row;
            nextPos->col = curPos.col;

            if(dir == up) nextPos->row -= 1;
            else if(dir == right) nextPos->col += 1;
            else if(dir == down) nextPos->row += 1;
            else if(dir == left) nextPos->col -= 1;
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

            char mapObj;
            Pos o_pos, x_pos;

            // delete 'o' in old position.
            o_pos.row = cur_state.row;
            o_pos.col = cur_state.col;
            mapObj = get_map_object(renderedMap, o_pos); 
            if(mapObj == 'o') set_map_object(renderedMap, o_pos, ' ');
            else if(mapObj == 'O') set_map_object(renderedMap, o_pos, '.');
            else {fprintf(stderr, "[act()] mapObj != 'o' \n"); exit(-1);}
            
            // replace 'x' in old position by 'o'.
            move(o_pos, action.dir, &x_pos);
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


            map2state(renderedMap, &next_state);
        }


        void set_map_object(char* map, Pos pos, char ch){
            map[rowBegins[pos.row] + pos.col] = ch;
        }


        void map2state(char* renderedMap, State* state){
            char ch;

            state.boxPos = 0b0;
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
                        state.boxPos.set(size_t(offset++), true);
                    }
                    else if(ch == 'o' || ch == 'O'){
                        offset++;
                        state.row = row;
                        state.col = col;
                    }       
                }             

                col++;
            }      
        }


        void is_reachable(State state, Pos curPos, Pos visitedPos){

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
        map->print_map();
        map->print_map_state(); 

        nextStateQueue.push(map.get_state());
    }


    void explore(){

        queue<State> nextStateQueue;

        unordered_map<bitset<64>, Pos> visitedState;
        unordered_map<bitset<64>, PosNode*> visitedCollidedState;

        State state, nextState;
        list<Action> action_list;
        // Pos pos;
        
        bool done = false;

        while (!done) {
            state = nextStateQueue.front();
            nextStateQueue.pop();

            add_visited_state(&visitedState, &visitedCollidedState, state);


            get_available_actions(state, &action_list);

            while(!action_list.empty()){
                
                map->act(state, action_list.front(), &nextState);
                action_list.pop_front()
                
                if(map->is_done(nextState)){done = true; break;} 

                if(!is_state_visited(&visitedState, &visitedCollidedState, nextState)){
                    nextStateQueue.push(nextState);
                }
            }
        }
    }


    void add_visited_state(unordered_map<bitset<64>, Pos> *visitedState, 
                    unordered_map<bitset<64>, PosNode*> *visitedCollidedState, State state){
        
        Pos pos;
        pos.row = state.row;
        pos.col = state.col;

        if(!is_in_visitedState(&visitedState, state.boxPos)){ // not inside.
            (*visitedState)[state.boxPos] = pos;
        }else{
            insert_PosNode(&visitedCollidedState, state);
        }
    }


    void insert_PosNode(unordered_map<bitset<64>, PosNode> *visitedCollidedState, State state){
        
        Pos pos;
        pos.row = state.row;
        pos.col = state.col;

        PosNode *cur = new PosNode;
        cur->pos = pos;

        if(!is_in_visitedCollidedState(&visitedCollidedState, state.boxPos)){
            cur->next = NULL;
        }
        else{
            cur->next = (*visitedCollidedState)[state.boxPos];
        }

        (*visitedCollidedState)[state.boxPos] = cur;
    }


    bool is_state_visited(unordered_map<bitset<64>, Pos> *visitedState, 
                    unordered_map<bitset<64>, PosNode*> *visitedCollidedState, State state){

        // key not found -> haven't been visited.
        if(!is_in_visitedState(&visitedState, state.boxPos)){return false;}


        Pos visitedPos = (*visitedState)[state.boxPos];
        Pos curPos{.row{state.row}, .col{state.col}};


        // Have same box positions, and mutually 
            // reachable player positions *with the head* -> deemed visited.
        if(map->is_reachable(state, curPos, visitedPos)){return true;}
        

        // key not found -> haven't been visited.
        if(!is_in_visitedCollidedState(&visitedCollidedState, state.boxPos)){return false;}


  
        PosNode *cur_ptr = (*visitedCollidedState)[state.boxPos];

        while(cur_ptr){

            visitedPos = cur_ptr->pos;

            // Have same box positions, and mutually 
                // reachable player positions *with other collided pos*. 
            if(map->is_reachable(state, curPos, visitedPos)){return true;}
            
            cur_ptr = cur_ptr->next;
        }

        return false;
    }
    

    bool is_in_visitedCollidedState(unordered_map<bitset<64>, PosNode*> *visitedCollidedState, 
                                                                            bitset<64> boxPos){
        return !(visitedCollidedState->find(boxPos) == visitedCollidedState->end());
    }


    bool is_in_visitedState(unordered_map<bitset<64>, Pos> *visitedState, bitset<64> boxPos){
        return !(visitedState->find(boxPos) == visitedState->end());
    }


    // struct StateKey{
    //     bitset<64> boxPos;
    //     unsigned char idx;
    // }

    struct PosNode{
        Pos pos;
        PosNode *next;
    }

    Map* map;
};





int main(int argc, char** argv) {
    
    fprintf(stderr, "argv[1]: %s\n", argv[1]);

    Solver solver(argv[1]);

    return 0;
}