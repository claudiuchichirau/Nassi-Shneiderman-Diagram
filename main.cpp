#include <iostream>
#include <fstream>
#include <graphics.h>
#include <winbgim.h>
#include <string>
#include <map>
#include <vector>
#include <windows.h>

using namespace std;

#define KEY_UP 72
#define KEY_DOWN 80

#define key_scroll_delta 50

#define screen_height 1080
#define screen_width 1920
#define inlineCharacter 32

#define sideMenuWidth 500

int bkg_colors[] = {14,2,3,6,9,11,10};
int get_bkg_color(int level){
    return bkg_colors[level % 6];
}

enum instruction_type{
    base,                   //done
    while_type,             //done
    do_while_type,          //done
    for_type,               //done
    if_type,                //done
    elif_type,              //done
    else_type,              //done
    code_start_type,        //nothing to be done
    function_type           //done
};

struct instruction{
    instruction_type type;
    instruction *next_instruction;
    string text_content;

    instruction(instruction_type _type){
        type = _type;
        next_instruction = NULL;
        text_content = "/";
    }
};
struct while_instruction : instruction{
    instruction *child_instruction;
    while_instruction() : instruction(while_type) {
        child_instruction = NULL;
    }
};
struct do_while_instruction : instruction{
    instruction *child_instruction;
    do_while_instruction() : instruction(do_while_type) {
        child_instruction = NULL;
    }
};
struct for_instruction : instruction{
    instruction *child_instruction;
    for_instruction() : instruction(for_type) {
        child_instruction = NULL;
    }
};
struct conditional_if_instruction : instruction{
    instruction *child_instruction;

    conditional_if_instruction(instruction_type _type = if_type) : instruction(_type){
        type = _type;
        child_instruction = NULL;
    }
};
struct function_instruction : instruction{
    instruction *child_instruction;
    function_instruction() : instruction(function_type){
        child_instruction = NULL;
    }
};

class StringProcessingUtils{
public:
    bool is_for_loop_valid(string &line){
        line = ltrim(line);
        if(line.length() <= 3) return false;
        if(line[0] == 'f' && line[1] == 'o' && line[2] == 'r')
            return true;
        return false;
    }

    bool is_if_valid(string &line){
        line = ltrim(line);
        if(line.length() <= 2) return false;
        if(line[0] == 'i' && line[1] == 'f')
            return true;
        return false;
    }

    bool is_else_valid(string &line){
        line = ltrim(line);
        if(line.length() != 4) return false;
        if(line[0] == 'e' && line[1] == 'l' && line[2] == 's' &&
           line[3] == 'e')
            return true;
        return false;
    }

    bool is_elif_valid(string &line){
        line = ltrim(line);
        if(line.length() <= 3) return false;
        if(line[0] == 'e' && line[1] == 'l' && line[2] == 'i' &&
           line[3] == 'f')
            return true;
        return false;
    }

    bool is_function_valid(string &line){
        line = ltrim(line);
        if(line.length() <= 7) return false;
        if(line[0] == 'f' && line[1] == 'u' && line[2] == 'n' &&
           line[3] == 'c' && line[4] == 't' && line[5] == 'i' &&
           line[6] == 'o' && line[7] == 'n')
            return true;
        return false;
    }

    bool is_valid_do_while(string &line){
        line = ltrim(line);
        if(line.length() != 2) return false;
        if(line[0] == 'd' && line[1] == 'o')
            return true;
        return false;
    }

    bool is_valid_while(string &line)
    {
        line = ltrim(line);
        if(line.length() <= 4) return false;
        if(line[0] == 'w' && line[1] == 'h' && line[2] == 'i' &&
           line[3] == 'l' && line[4] == 'e')
            return true;
        return false;
    }

    string ltrim(string &s)
    {
        size_t start = s.find_first_not_of(WHITESPACE);
        return (start == std::string::npos) ? "" : s.substr(start);
    }

    string rtrim(string &s)
    {
        size_t end = s.find_last_not_of(WHITESPACE);
        return (end == std::string::npos) ? "" : s.substr(0, end + 1);
    }

private:
    const string WHITESPACE = " \n\r\t\f\v";
};

class AlgorithmLoader{
public:
    vector<string> code_lines;

    instruction* loadAlgo(bool print = false){
        ifstream file("input.in");

        instruction* algorithm = new instruction(code_start_type);
        indentation_map.insert({0, algorithm});
        int last_element_indentation_level = 0;

        for(string line; getline(file, line); )
        {
            code_lines.push_back(line);
            if(has_only_spaces(line)) continue;

            int indentation_level = get_indentation_level(line);
            instruction* new_instruction = create_instruction_from_line(line);
            instruction* parent = get_indentation_mapped_instruction(indentation_level);

            if(parent == NULL || (indentation_level > last_element_indentation_level)){
                parent = get_indentation_mapped_instruction(last_element_indentation_level);
                switch(parent->type)
                {
                    case for_type:
                    case while_type:
                    case do_while_type:
                    case function_type:
                    {
                        for_instruction* for_parent = (for_instruction*)(parent);
                        for_parent->child_instruction = new_instruction;
                        break;
                    }
                    case if_type:
                    case else_type:
                    case elif_type:
                    {
                        conditional_if_instruction* if_parent = (conditional_if_instruction*)(parent);
                        if_parent->child_instruction = new_instruction;
                        break;
                    }
                    default:
                        break;
                }
            }
            else
            {
                if(parent->type == do_while_type && new_instruction -> type == while_type)
                {
                    parent->text_content =  new_instruction->text_content;
                    continue;
                }
                parent->next_instruction = new_instruction;
            }
            indentation_map[indentation_level] = new_instruction;
            last_element_indentation_level = indentation_level;
            if(print)
                cout<<indentation_level<<" | "<<line<<endl;
        }
        file.close();

        return algorithm;
    }

    void preview_algo(instruction* line, int level = 0){
        if(line == NULL)
            return;
        cout<<line->type<<" | ";
        for(int i = 0; i < level; i++)
            cout<<"-";
        cout<<line->text_content<<endl;
        if(line->type == for_type || line-> type == function_type || line -> type == while_type || line -> type == do_while_type)
        {
            for_instruction* for_parent = (for_instruction*)line;
            preview_algo(for_parent->child_instruction, level + 1);
        }
        if(line -> type == if_type || line -> type == elif_type || line -> type == else_type)
        {
            cout<<"->"<<endl;
            conditional_if_instruction* if_parent = (conditional_if_instruction*)line;
            preview_algo(if_parent->child_instruction, level + 1);
        }
        if(line->next_instruction != NULL)
            preview_algo(line->next_instruction, level);
    }
private:
    map<int, instruction*> indentation_map;
    StringProcessingUtils spu;

    instruction* create_instruction_from_line(string& line)
    {
        if(spu.is_for_loop_valid(line))
        {
            instruction* to_return = new for_instruction();
            to_return->text_content = spu.ltrim(line);
            return to_return;
        }
        if(spu.is_if_valid(line))
        {
            instruction* to_return = new conditional_if_instruction();
            line = line.substr(2);
            to_return->text_content = spu.ltrim(line);
            return to_return;
        }
        if(spu.is_else_valid(line))
        {
            instruction *to_return = new conditional_if_instruction(else_type);
            to_return->text_content = spu.ltrim(line);
            return to_return;
        }
        if(spu.is_elif_valid(line))
        {
            instruction *to_return = new conditional_if_instruction(elif_type);
            line = line.substr(4);
            to_return->text_content = spu.ltrim(line);
            return to_return;
        }
        if(spu.is_function_valid(line))
        {
            instruction* to_return = new function_instruction();
            to_return->text_content = spu.ltrim(line);
            return to_return;
        }
        if(spu.is_valid_while(line))
        {
            instruction* to_return = new while_instruction();
            to_return->text_content = spu.ltrim(line);
            return to_return;
        }
        if(spu.is_valid_do_while(line))
        {
            instruction* to_return = new do_while_instruction();
            to_return->text_content = spu.ltrim(line);
            return to_return;
        }

        instruction* to_return_base = new instruction(base);
        to_return_base->text_content = line;
        return to_return_base;
    }

    instruction* get_indentation_mapped_instruction(int level){
        if(indentation_map.count(level))
            return indentation_map.at(level);
        else
            return NULL;
    }

    int get_indentation_level(string line){
        int st = 0;
        while(line[st] == inlineCharacter)
            st++;
        return st;
    }

    bool has_only_spaces(string& line) {
        return line.find_first_not_of (' ') == line.npos;
    }
};

int drawElement(instruction* line, int top, int left, int right, int level);

int drawBaseCommand(instruction* line, int top, int left, int right, int level){
    char* str = (char*)line->text_content.c_str();
    int text_w = textwidth(str);
    int text_h = textheight(str);
    int rect_w = right - left;
    int rect_h = text_h + 10;

    int x = left + (rect_w - text_w) / 2;
    int y = top + (rect_h - text_h) / 2;

    int bottom = rect_h + top;

    outtextxy(x, y, str);
    rectangle(left, top, right, bottom);
    return bottom;
}
int drawForCommand(instruction* line, int top, int left, int right, int level){
    char* str = (char*)line->text_content.c_str();
    int text_w = textwidth(str);
    int text_h = textheight(str);
    int rect_w = right - left;
    int rect_h = text_h + 10;

    int x = left + (rect_w - text_w) / 2;
    int y = top + (rect_h - text_h) / 2;

    int bottom = rect_h + top;

    int bkg_color = get_bkg_color(level);

    setcolor(bkg_color);
    rectangle(left, top, right, bottom);

    setfillstyle(SOLID_FILL, bkg_color);
    floodfill(left+1, top+1, bkg_color);

    setcolor(WHITE);

    for_instruction* for_element = (for_instruction*)(line);
    if(for_element->child_instruction != NULL){
        bottom = drawElement(for_element->child_instruction, bottom+1, left + 30, right, level + 1);
    }

    setcolor(bkg_color);
    rectangle(left, top, left + 30, bottom);
    setfillstyle(SOLID_FILL, bkg_color);
    floodfill(left+1, top+rect_h+1, bkg_color);
    setcolor(WHITE);
    outtextxy(x, y, str);

    return bottom;
}
int drawDoWhileCommand(instruction* line, int top, int left, int right, int level){
    char* str = (char*)line->text_content.c_str();
    int text_w = textwidth(str);
    int text_h = textheight(str);
    int rect_w = right - left;
    int rect_h = text_h + 10;

    int bottom = 0;

    int bkg_color = get_bkg_color(level);

    setcolor(WHITE);

    for_instruction* for_element = (for_instruction*)(line);
    if(for_element->child_instruction != NULL){
        bottom = drawElement(for_element->child_instruction, top, left + 30, right, level + 1);
    }

    setcolor(bkg_color);
    rectangle(left, top, left + 30, bottom);
    setfillstyle(SOLID_FILL, bkg_color);
    floodfill(left+1, top+1, bkg_color);

    setcolor(bkg_color);
    rectangle(left, bottom, right, rect_h + bottom);
    setfillstyle(SOLID_FILL, bkg_color);
    floodfill(left+1, bottom+1, bkg_color);

    bottom = rect_h + bottom;
    int x = left + (rect_w - text_w) / 2;
    int y = bottom - text_h/2 - 10 - 1;

    setcolor(WHITE);
    outtextxy(x, y, str);

    return bottom;
}
int drawFunctionCommand(instruction* line, int top, int left, int right, int level){
    char* str = (char*)line->text_content.c_str();
    int text_w = textwidth(str);
    int text_h = textheight(str);
    int rect_w = right - left;
    int rect_h = text_h + 10;

    int x = left + (rect_w - text_w) / 2;
    int y = top + (rect_h - text_h) / 2;

    int bottom = rect_h + top;

    int bkg_color = get_bkg_color(level);

    setcolor(bkg_color);
    rectangle(left, top, right, bottom);

    setfillstyle(SOLID_FILL, bkg_color);
    floodfill(left+1, top+1, bkg_color);

    setcolor(WHITE);

    for_instruction* for_element = (for_instruction*)(line);
    if(for_element->child_instruction != NULL){
        bottom = drawElement(for_element->child_instruction, bottom+1, left + 30, right - 30, level + 1);
    }

    setcolor(bkg_color);
    rectangle(left, top, left + 30, bottom);
    setfillstyle(SOLID_FILL, bkg_color);
    floodfill(left+1, top+rect_h+1, bkg_color);
    setcolor(WHITE);
    outtextxy(x, y, str);

    setcolor(bkg_color);
    rectangle(left, bottom, right, rect_h + bottom);
    setfillstyle(SOLID_FILL, bkg_color);
    floodfill(left+1, bottom+1, bkg_color);

    rectangle(right-30, top, right, rect_h + bottom);
    setfillstyle(SOLID_FILL, bkg_color);
    floodfill(right-30+1, top+rect_h+1, bkg_color);

    bottom += rect_h;

    setcolor(WHITE);
    return bottom;
}
int drawIfCommand(instruction* command, int top, int left, int right, int level){
    if(command->next_instruction->type != elif_type){
        char* str = (char*)command->text_content.c_str();
        int text_w = textwidth(str);
        int text_h = textheight(str);
        int rect_w = right - left;
        int rect_h = text_h * 3 + 10;

        int x = left + (rect_w - text_w) / 2;
        int y = top + (rect_h - text_h) / 2;

        int bottom = rect_h + top;

        outtextxy(x, y, str);
        rectangle(left, top, right, bottom);

        line(left, top, left + (right - left) / 2, bottom);
        line(right, top, left + (right - left) / 2, bottom);

        outtextxy(left + 10, y, "TRUE");
        outtextxy(right - 10 - textwidth("FALSE"), y, "FALSE");

        int true_bottom = bottom;
        int false_bottom = bottom;

        conditional_if_instruction* if_command = (conditional_if_instruction*)(command);
        if(if_command->child_instruction != NULL){
            true_bottom = drawElement(if_command->child_instruction, bottom, left, left + (right - left) / 2, level+1);
        }
        if(if_command->next_instruction->type == else_type){
            conditional_if_instruction* else_command = (conditional_if_instruction*)(if_command->next_instruction);
            if(else_command->child_instruction != NULL){
                false_bottom = drawElement(else_command->child_instruction, bottom, left + (right - left) / 2, right, level + 1);
            }
        }

        if(true_bottom > false_bottom)
            bottom = true_bottom;
        else
            bottom = false_bottom;
        return bottom-1;
    }
    else{
        char* str = "IF";
        int text_w = textwidth(str);
        int text_h = textheight(str);
        int rect_w = right - left;
        int rect_h = text_h + 10;

        int x = left + (rect_w - text_w) / 2;
        int y = top + (rect_h - text_h) / 2;

        int bottom = rect_h + top;

        int bkg_color = get_bkg_color(level);

        rectangle(left, top, right, bottom);
        outtextxy(x, y, str);

        int conditions_count = 0;
        conditional_if_instruction* conditions[100];
        conditional_if_instruction* p = (conditional_if_instruction*)command;
        while(p->type == elif_type || p->type == else_type || p->type == if_type){
            conditions[conditions_count++] = p;
            if(p->next_instruction != NULL)
                p = (conditional_if_instruction*)(p->next_instruction);
            else
                break;
        }
        int max_bottom_conditions = 0;
        for(int i = 0; i < conditions_count; i++){

            int _c_left = left + ((right - left) / conditions_count) * i;
            int _c_right = right - ((right - left) / conditions_count) * (conditions_count - i - 1);

            char* _str = (char*)conditions[i]->text_content.c_str();
            int text_w = textwidth(str);
            int text_h = textheight(str);
            int rect_w = _c_right - _c_left;
            int rect_h = text_h + 10;

            int x = _c_left + (rect_w - text_w) / 2;
            int y = bottom + (rect_h - text_h) / 2;

            //setcolor(bkg_color);
            rectangle(_c_left, bottom, _c_right, bottom + rect_h);
            //setfillstyle(SOLID_FILL, bkg_color);
            //floodfill(_c_left+1, bottom+1, bkg_color);
            setcolor(WHITE);
            outtextxy(x, y, _str);

            if(conditions[i]->child_instruction != NULL)
            {
                int condition_bottom = drawElement(conditions[i]->child_instruction, bottom + rect_h, _c_left, _c_right, level+1);
                if(condition_bottom > max_bottom_conditions)
                    max_bottom_conditions = condition_bottom;

            }
        }
        bottom = max_bottom_conditions;

        return bottom;
    }
    return top;
}

int drawElement(instruction* line, int top, int left, int right, int level)
{
    int bottom = top;
    switch(line->type){
        case base: {
            bottom = drawBaseCommand(line, top, left, right, level);
            break;
        }
        case for_type:
        case while_type: {
            bottom = drawForCommand(line, top, left, right, level);
            break;
        }
        case do_while_type:{
            bottom = drawDoWhileCommand(line, top, left, right, level);
            break;
        }
        case function_type: {
            bottom = drawFunctionCommand(line, top, left, right, level);
            break;
        }
        case if_type: {
            bottom = drawIfCommand(line, top, left, right, level);
            break;
        }
    }

    if(line->next_instruction != NULL){
        bottom = drawElement(line->next_instruction, bottom, left, right, level);
    }

    return bottom;
}

void drawButton(int left, int top, int right, int bottom, string buttonText){
    char* str = (char*)buttonText.c_str();
    int text_w = textwidth(str);
    int text_h = textheight(str);
    int rect_w = right - left;
    int rect_h = bottom - top;

    int x = left + (rect_w - text_w) / 2;
    int y = top + (rect_h - text_h) / 2;

    rectangle(left, top, right, bottom);
    rectangle(left+5, top+5, right-5, bottom-5);
    outtextxy(x,y,str);
}

void drawUI(int height_delta, instruction* algorithm, vector<string> lines){
    setcolor(WHITE);
    rectangle(5, 5, sideMenuWidth-5, screen_height-5);
    rectangle(10, 10, sideMenuWidth-10, screen_height-10);
    setfillstyle(SOLID_FILL, WHITE);
    floodfill(6, 6, WHITE);

    drawButton(15, 15, (sideMenuWidth-15) / 2, 15+35, "Urca");
    drawButton((sideMenuWidth-15) / 2, 15, (sideMenuWidth-15),15+35, "Coboara");

    outtextxy(15,60, "Previzualizare cod:");
    for(int i = 0; i < lines.size(); i++)
    {
        outtextxy(15,80 + i*20, (char*)lines[i].c_str());
    }

    drawElement(algorithm ->next_instruction, 5 + height_delta, sideMenuWidth, screen_width - 5, 0);
}

void launchWinbgimPreview(instruction* algorithm, vector<string> lines){
    initwindow(screen_width, screen_height);
    int height_delta = 0;

    drawUI(height_delta, algorithm, lines);

    POINT cursorPosition;
    int mX, mY;

    while(true){
        bool changed = false;
        if(GetAsyncKeyState(VK_LBUTTON))
        {
            GetCursorPos(&cursorPosition);
            ScreenToClient(GetForegroundWindow(), &cursorPosition);
            mX = cursorPosition.x;
            mY = cursorPosition.y;

            if(15 <= mY && mY <= 50){
                if(15 <= mX && mX <= (sideMenuWidth-15) / 2){
                    height_delta+=key_scroll_delta;
                    changed = true;
                }
                else if((sideMenuWidth-15) / 2 <= mX && mX <= sideMenuWidth-15){
                    height_delta-=key_scroll_delta;
                    changed = true;
                }
            }
        }
        if(kbhit())
        {
            switch(getch()) {
                case KEY_UP:
                    height_delta+=key_scroll_delta;
                    changed = true;
                    break;
                case KEY_DOWN:
                    height_delta-=key_scroll_delta;
                    changed = true;
                    break;
                default:
                    break;
            }
        }
        if(changed)
        {
            cleardevice();
            drawUI(height_delta, algorithm, lines);
        }
        delay(5);
    }

    getch(); closegraph();
}

int main()
{
    AlgorithmLoader loader;
    instruction* algo = loader.loadAlgo();
    loader.preview_algo(algo);

    launchWinbgimPreview(algo, loader.code_lines);

    return 0;
}
