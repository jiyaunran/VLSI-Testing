/* Logic Simulator
 * Last update: 2006/09/20 */
#include <iostream>
#include "gate.h"
#include "circuit.h"
#include "ReadPattern.h"
#include "GetLongOpt.h"
using namespace std;

extern GetLongOpt option;

// Ass3
#include <algorithm>

//do logic simulation for test patterns
void CIRCUIT::LogicSimVectors()
{
    // cout << "Run logic simulation" << endl;
    evaluate_num = 0;
    //read test patterns
    while (!Pattern.eof()) {
        Pattern.ReadNextPattern();
        SchedulePI();
        LogicSim();
        PrintIO();
    }
    return;

}
// modified
void CIRCUIT::ModLogicSimVectors()
{
    cout << "Run modified logic simulation" << endl;
    //read test patterns
    while (!Pattern.eof()) {
        Pattern.ReadNextPattern();
        SchedulePI();
        ModLogicSim();
        PrintIO();
    }
    return;
}

void CIRCUIT::ModLogicSim()
{
    GATE* gptr;
    VALUE new_value;
    for (unsigned i = 0;i <= MaxLevel;i++) {
        while (!Queue[i].empty()) {
            gptr = Queue[i].front();
            Queue[i].pop_front();
            gptr->ResetFlag(SCHEDULED);
            new_value = ModEvaluate(gptr);
            if (new_value != gptr->GetValue()) {
                gptr->SetValue(new_value);
                ScheduleFanout(gptr);
            }
        }
    }
    return;
}

VALUE CIRCUIT::NandFunction(GATEPTR gptr){
    GATEFUNC fun(gptr->GetFunction());
    VALUE cv(CV[fun]); //controling value
    VALUE value;
    bool unknown = 0;

    // cout << fun << endl;
    for (unsigned i = 0;i<gptr->No_Fanin();++i) {
        value = gptr->Fanin(i)->GetValue();
        // cout << "i: " << i << " value: " << value << endl;
        if(value == cv) { 
            // controlled value case
            value = S0;
            return value;
        }
        else if(value == X) {
            unknown = 1;
            // cout << "unknown" << endl;
        }
    }
    // cout << "unknown: " << unknown << " value: " << value << endl;
    if (unknown) return X;
    else return S1;
}

VALUE CIRCUIT::NorFunction(GATEPTR gptr){
    GATEFUNC fun(gptr->GetFunction());
    VALUE cv(CV[fun]); //controling value
    VALUE value;
    bool unknown = 0;

    // cout << fun << endl;
    for (unsigned i = 0;i<gptr->No_Fanin();++i) {
        value = gptr->Fanin(i)->GetValue();
        // cout << "i: " << i << " value: " << value << " unknown: " << unknown << endl;
        if(value == cv) { 
            // controlled value case
            value = S1;
            return value;
        }
        else if(value == X) {
            unknown = 1;
            // cout << "unknown" << endl;
        }
    }
    // cout << "unknown: " << unknown << " value: " << value << endl;
    if (unknown) return X;
    else return S0;
}

VALUE CIRCUIT::NotFunction(VALUE v){
    if (v==X) return X;
    else if (v == S0) return S1;
    else return S0;
}

VALUE CIRCUIT::ModEvaluate(GATEPTR gptr)
{
    // cout << "S0 S1 X: " << S0 << ' ' << S1 << ' ' << X << endl;
    GATEFUNC fun(gptr->GetFunction());
    VALUE cv(CV[fun]); //controling value
    VALUE value(gptr->Fanin(0)->GetValue());
    // cout << "fun: " << fun << endl;
    switch (fun) {
        case G_AND:
        case G_NAND:
            value = NandFunction(gptr);
            break;
        case G_OR:
        case G_NOR:
            value = NorFunction(gptr);
            break;
        default: break;
    }
    //NAND, NOR and NOT
    // cout << "before not value: " << value << " Is inversion: " << gptr->Is_Inversion() << endl;
    if (gptr->Is_Inversion()) {value = NotFunction(value);}
    // VALUE tmp;
    // for (unsigned i = 0;i<gptr->No_Fanin();++i){        
    //     tmp = gptr->Fanin(i)->GetValue();
    //     cout << "i: " << i << " value: " << tmp << endl;
    // }
    // cout << "end value: " << value << endl;
    return value;
}


//do event-driven logic simulation
void CIRCUIT::LogicSim()
{
    GATE* gptr;
    VALUE new_value;
    for (unsigned i = 0;i <= MaxLevel;i++) {
        while (!Queue[i].empty()) {
            gptr = Queue[i].front();
            Queue[i].pop_front();
            gptr->ResetFlag(SCHEDULED);
            new_value = Evaluate(gptr);
            if (new_value != gptr->GetValue()) {
                gptr->SetValue(new_value);
                ScheduleFanout(gptr);
            }
        }
    }
    return;
}

//Used only in the first pattern
void CIRCUIT::SchedulePI()
{
    for (unsigned i = 0;i < No_PI();i++) {
        if (PIGate(i)->GetFlag(SCHEDULED)) {
            PIGate(i)->ResetFlag(SCHEDULED);
            ScheduleFanout(PIGate(i));
        }
    }
    return;
}

//schedule all fanouts of PPIs to Queue
void CIRCUIT::SchedulePPI()
{
    for (unsigned i = 0;i < No_PPI();i++) {
        if (PPIGate(i)->GetFlag(SCHEDULED)) {
            PPIGate(i)->ResetFlag(SCHEDULED);
            ScheduleFanout(PPIGate(i));
        }
    }
    return;
}

//set all PPI as 0
void CIRCUIT::SetPPIZero()
{
    GATE* gptr;
    for (unsigned i = 0;i < No_PPI();i++) {
        gptr = PPIGate(i);
        if (gptr->GetValue() != S0) {
            gptr->SetFlag(SCHEDULED);
            gptr->SetValue(S0);
        }
    }
    return;
}

//schedule all fanouts of gate to Queue
void CIRCUIT::ScheduleFanout(GATE* gptr)
{
    for (unsigned j = 0;j < gptr->No_Fanout();j++) {
        Schedule(gptr->Fanout(j));
    }
    return;
}

//initial Queue for logic simulation
void CIRCUIT::InitializeQueue()
{
    SetMaxLevel();
    Queue = new ListofGate[MaxLevel + 1];
    return;
}

//evaluate the output value of gate
VALUE CIRCUIT::Evaluate(GATEPTR gptr)
{
    GATEFUNC fun(gptr->GetFunction());
    VALUE cv(CV[fun]); //controling value
    VALUE value(gptr->Fanin(0)->GetValue());
    evaluate_num++;
    // cout << "func: " << fun << endl;
    switch (fun) {
        case G_AND:
        case G_NAND:
            for (unsigned i = 1;i<gptr->No_Fanin() && value != cv;++i) {
                value = AndTable[value][gptr->Fanin(i)->GetValue()];
                //cout << value << endl;
            }
            break;
        case G_OR:
        case G_NOR:
            for (unsigned i = 1;i<gptr->No_Fanin() && value != cv;++i) {
                value = OrTable[value][gptr->Fanin(i)->GetValue()];
            }
            break;
        default: break;
    }
    //NAND, NOR and NOT
    if (gptr->Is_Inversion()) { value = NotTable[value]; }
    // cout << "end value: " << value << endl;
    return value;
}

extern GATE* NameToGate(string);

void PATTERN::Initialize(char* InFileName, int no_pi, string TAG)
{
    patterninput.open(InFileName, ios::in);
    if (!patterninput) {
        cout << "Unable to open input pattern file: " << InFileName << endl;
        exit( -1);
    }
    string piname;
    while (no_pi_infile < no_pi) {
        patterninput >> piname;
        if (piname == TAG) {
            patterninput >> piname;
            inlist.push_back(NameToGate(piname));
            no_pi_infile++;
        }
        else {
            cout << "Error in input pattern file at line "
                << no_pi_infile << endl;
            cout << "Maybe insufficient number of input\n";
            exit( -1);
        }
    }
    return;
}

//Assign next input pattern to PI
void PATTERN::ReadNextPattern()
{
    char V;
    for (int i = 0;i < no_pi_infile;i++) {
        patterninput >> V;
        if (V == '0') {
            if (inlist[i]->GetValue() != S0) {
                inlist[i]->SetFlag(SCHEDULED);
                inlist[i]->SetValue(S0);
            }
        }
        else if (V == '1') {
            if (inlist[i]->GetValue() != S1) {
                inlist[i]->SetFlag(SCHEDULED);
                inlist[i]->SetValue(S1);
            }
        }
        else if (V == 'X') {
            if (inlist[i]->GetValue() != X) {
                inlist[i]->SetFlag(SCHEDULED);
                inlist[i]->SetValue(X);
            }
        }
    }
    //Take care of newline to force eof() function correctly
    patterninput >> V;
    if (!patterninput.eof()) patterninput.unget();
    return;
}

void CIRCUIT::PrintIO()
{
    register unsigned i;
    // for (i = 0;i<No_PI();++i) { cout << PIGate(i)->GetValue(); }
    // cout << " ";
    // for (i = 0;i<No_PO();++i) { cout << POGate(i)->GetValue(); }
    // cout << endl;
    return;
}

// ASS3
void CIRCUIT::ParInitPrint(string simulator_name, string benchfile_path){
    simulator.open(simulator_name);
    const int pattern_num = 16;
    string init_lib = "#include <iostream>\n#include <ctime>\n#include <bitset>\n#include <string>\n#include <fstream>\n\nusing namespace std;\n";
    simulator << init_lib;
    simulator << "const unsigned PatternNum = " << pattern_num << ";\nvoid evaluate();\nvoid printIO(unsigned idx);\n\n";
    for(unsigned int i=0; i<No_Gate(); i++){
        simulator << "bitset<PatternNum> G_" << Gate(i)->GetName() << "[2];\n";
    }
    simulator << "bitset<PatternNum> temp;\n";
    //cout << benchfile_path << endl;
    
    string file_out_name = retreive_path_name(benchfile_path);
    // cout << file_out_name << endl;
    simulator << "ofstream fout(\"" << file_out_name << ".out\",ios::out);\n\n";

    simulator << "int main(){\n";
    simulator << "\tclock_t time_init, time_end;\n";
    simulator << "\ttime_init = clock();\n";
    ifstream fin;
    fin.open(option.retrieve("input"));
    // int **input_value = new int *[pattern_num];
    // for(int i=0; i < pattern_num; i++){
    //     input_value[i] = new int [pattern_num];
    // }

    // CONVERT INPUT PATTERN TO BITSET

    string *input_value_unit = new string [pattern_num];
    string *input_value_tens = new string [pattern_num];
    string line;
    getline(fin, line);
    int loop_pat_num = 0;
    while(getline(fin, line)){
        
        line.erase(remove(line.begin(), line.end(), ' '), line.end());
        line.erase(remove(line.begin(), line.end(), '\n'), line.end());
        // for(int j=0; j<line.length(); j++){
        //     if (line[j] != ' ') cout << line[j];
        // }
        // cout << endl;
        for(int j=0; j<line.length(); j++){
            if (line[j] == '0'){
                input_value_unit[loop_pat_num] += '0';
                input_value_tens[loop_pat_num] += '0';
            }
            else if (line[j] == '1'){
                input_value_unit[loop_pat_num] += '1';
                input_value_tens[loop_pat_num] += '1';
            }
            else if (line[j] == 'X' || line[j] == 'x'){
                input_value_unit[loop_pat_num] += '1';
                input_value_tens[loop_pat_num] += '0';
            }
        }
        
        if (loop_pat_num == (pattern_num-1)){
            for(unsigned int i=0; i<No_PI(); i++){
                bitset<pattern_num> bits[2];
                for(int j=0; j<pattern_num; j++){
                    bits[0][j] = input_value_unit[j][i] - '0';
                    bits[1][j] = input_value_tens[j][i] - '0';
                }

                simulator << "\tG_" << PIGate(i)->GetName() << "[0] = " << bits[0].to_ullong() << " ;\n";
                simulator << "\tG_" << PIGate(i)->GetName() << "[1] = " << bits[1].to_ullong() << " ;\n";
            }
            simulator << "\n\tevaluate();\n";
            simulator << "\tprintIO(" << pattern_num << ");\n\n";

            loop_pat_num = 0;
            for (int i=0; i<pattern_num; i++){
                input_value_unit[i] = "";
                input_value_tens[i] = "";
            }
        }
        else loop_pat_num++;
    }

    if (loop_pat_num != 0){
        for(unsigned int i=0; i<No_PI(); i++){
            bitset<pattern_num> bits[2];
            bits[0].reset();
            bits[1].reset();
            for(int j=0; j<loop_pat_num; j++){
                bits[0][j] = input_value_unit[j][i] - '0';
                bits[1][j] = input_value_tens[j][i] - '0';
            }

            simulator << "\tG_" << PIGate(i)->GetName() << "[0] = " << bits[0].to_ullong() << " ;\n";
            simulator << "\tG_" << PIGate(i)->GetName() << "[1] = " << bits[1].to_ullong() << " ;\n";
        }
        simulator << "\n\tevaluate();\n";
        simulator << "\tprintIO(" << loop_pat_num << ");\n\n";
    }


    simulator << "\ttime_end = clock();\n";
    simulator << "\tcout << \"Total CPU Time = \" << double(time_end - time_init)/CLOCKS_PER_SEC << endl;\n";
    simulator << "\tsystem(\"ps aux | grep a.out \");\n";
    simulator << "\treturn 0;\n";
    simulator << "}\n\n";

    simulator << "void evaluate()\n";
    simulator << "{\n";

    // evaluate function
    for(unsigned int i=0; i<MaxLevel; i++){
        for(unsigned int j=0; j<No_Gate(); j++){
            if(Gate(j)->GetLevel() == i)  {
                PrintGate(Gate(j));
                // cout << Gate(j)->GetName() << ' ' << Gate(j)->GetLevel() << endl;
            }
        }        
    }
    for (unsigned int i=0; i<No_PO(); i++){
        simulator << "\tG_" << POGate(i)->GetName() << "[0] = G_" << POGate(i)->GetInput_list()[0]->GetName() << "[0] ;\n"; 
        simulator << "\tG_" << POGate(i)->GetName() << "[1] = G_" << POGate(i)->GetInput_list()[0]->GetName() << "[1] ;\n";
    }
    simulator << "}\n";
    // cout << MaxLevel << endl;
    //ModParLogicSimVectors();


    // for (int i=0; i<No_Gate(); i++){
    //     // ignore 0 input gate
    //     if(Circuit.Gate(i)->GetInput_list().size() == 0) continue;
    //     simulator << "\tG_" << Circuit.Gate(i)->GetName() << "[0] = G_" << Circuit.Gate(i)->GetInput_list()[0]->GetName() << "[0] ;\n"; 
    //     simulator << "\tG_" << Circuit.Gate(i)->GetName() << "[1] = G_" << Circuit.Gate(i)->GetInput_list()[0]->GetName() << "[1] ;\n";
    //     switch (Circuit.Gate(i)->GetFunction())
    //     {
    //     case G_AND:
    //     case G_NAND:
    //         for(int j=1; j<Circuit.Gate(i)->No_Fanin() ;j++){
    //             cout << Circuit.Gate(i)->GetInput_list()[j]->GetName() << endl;
    //             simulator << "\tG_" << Circuit.Gate(i)->GetName() << "[0] &= G_" << Circuit.Gate(i)->GetInput_list()[j]->GetName() << "[0] ;\n";
    //             simulator << "\tG_" << Circuit.Gate(i)->GetName() << "[1] &= G_" << Circuit.Gate(i)->GetInput_list()[j]->GetName() << "[1] ;\n";
    //         }
    //         break;
    //     case G_OR:
    //     case G_NOR:
    //         for(int j=1; j<Circuit.Gate(i)->No_Fanin() ;j++){
    //             simulator << "\tG_" << Circuit.Gate(i)->GetName() << "[0] |= G_" << Circuit.Gate(i)->GetInput_list()[j]->GetName() << "[0] ;\n";
    //             simulator << "\tG_" << Circuit.Gate(i)->GetName() << "[1] |= G_" << Circuit.Gate(i)->GetInput_list()[j]->GetName() << "[1] ;\n";
    //         }
    //         break;
    //     default:
    //         break;
    //     }
    //     if (Circuit.Gate(i)->Is_Inversion()){
    //         simulator << "\ttemp = G_" << Circuit.Gate(i)->GetName() << "[0] ;\n";
    //         simulator << "\tG_" << Circuit.Gate(i)->GetName() << "[0] = ~G_" << Circuit.Gate(i)->GetName() << "[1] ;\n";
    //         simulator << "\tG_" << Circuit.Gate(i)->GetName() << "[1] = ~temp ;\n";
    //     }
    // }
    // simulator << "}\n";

    //printIO function
    simulator << "void printIO(unsigned idx){\n";
    simulator << "\tfor (unsigned j=0; j<idx; j++){\n";
    for(unsigned int i=0; i<No_PI(); i++){
        simulator << "\t\tif(G_" << PIGate(i)->GetName() << "[0][j]==0)\n";
        simulator << "\t\t\tif(G_" << PIGate(i)->GetName() << "[1][j]==1)\n";
        simulator << "\t\t\t\tfout << \"F\";\n";
        simulator << "\t\t\telse\n";
        simulator << "\t\t\t\tfout << \"0\";\n";
        simulator << "\t\telse{\n";
        simulator << "\t\t\tif(G_" << PIGate(i)->GetName() << "[1][j]==1)\n";
        simulator << "\t\t\t\tfout << \"1\";\n";
        simulator << "\t\t\telse\n";
        simulator << "\t\t\t\tfout << \"2\";\n";
        simulator << "\t\t}\n";
    }

    simulator << "\tfout << \" \";\n";

    for(unsigned int i=0; i<No_PO(); i++){
        simulator << "\t\tif(G_" << POGate(i)->GetName() << "[0][j]==0)\n";
        simulator << "\t\t\tif(G_" << POGate(i)->GetName() << "[1][j]==1)\n";
        simulator << "\t\t\t\tfout << \"F\";\n";
        simulator << "\t\t\telse\n";
        simulator << "\t\t\t\tfout << \"0\";\n";
        simulator << "\t\telse{\n";
        simulator << "\t\t\tif(G_" << POGate(i)->GetName() << "[1][j]==1)\n";
        simulator << "\t\t\t\tfout << \"1\";\n";
        simulator << "\t\t\telse\n";
        simulator << "\t\t\t\tfout << \"2\";\n";
        simulator << "\t\t}\n";
    }

    simulator << "\tfout << endl;\n";
    simulator << "\t}\n";
    simulator << '}';


    simulator.close();
}

string CIRCUIT::retreive_path_name(string path){
    int l = path.length();
    string bench_name = "";
    for(int i=0; i<l; i++){
        if (path[i] == '/') bench_name = "";
        else                bench_name += path[i];
    }
    for(int i=0; i<6; i++){
        bench_name.pop_back();
    }
    return bench_name;
}

void CIRCUIT::PrintGate(GATE* gptr){
    if(gptr->GetInput_list().size() != 0){
        simulator << "\tG_" << gptr->GetName() << "[0] = G_" << gptr->GetInput_list()[0]->GetName() << "[0] ;\n"; 
        simulator << "\tG_" << gptr->GetName() << "[1] = G_" << gptr->GetInput_list()[0]->GetName() << "[1] ;\n";
        //cout << gptr->GetInput_list().size() << endl;

        switch (gptr->GetFunction()){
            case G_AND:
            case G_NAND:
                for(unsigned int j=1; j<gptr->No_Fanin() ;j++){
                    //cout << gptr->GetInput_list()[j]->GetName() << endl;
                    simulator << "\tG_" << gptr->GetName() << "[0] &= G_" << gptr->GetInput_list()[j]->GetName() << "[0] ;\n";
                    simulator << "\tG_" << gptr->GetName() << "[1] &= G_" << gptr->GetInput_list()[j]->GetName() << "[1] ;\n";
                }
                break;
            case G_OR:
            case G_NOR:
                for(unsigned int j=1; j<gptr->No_Fanin() ;j++){
                    simulator << "\tG_" << gptr->GetName() << "[0] |= G_" << gptr->GetInput_list()[j]->GetName() << "[0] ;\n";
                    simulator << "\tG_" << gptr->GetName() << "[1] |= G_" << gptr->GetInput_list()[j]->GetName() << "[1] ;\n";
                }
                break;
            default:
                break;
        }
        if (gptr->Is_Inversion()){
            simulator << "\ttemp = G_" << gptr->GetName() << "[0] ;\n";
            simulator << "\tG_" << gptr->GetName() << "[0] = ~G_" << gptr->GetName() << "[1] ;\n";
            simulator << "\tG_" << gptr->GetName() << "[1] = ~temp ;\n";
        }
    }
}