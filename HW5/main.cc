#include <iostream>
#include <ctime>
#include "circuit.h"
#include "GetLongOpt.h"
#include "ReadPattern.h"

// Added library
#include "string.h"
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>

using namespace std;

// All defined in readcircuit.l
extern char* yytext;
extern FILE *yyin;
extern CIRCUIT Circuit;
extern int yyparse (void);
extern bool ParseError;

extern void Interactive();

GetLongOpt option;

// added function for hw1
void find_path(unsigned int cur_ID, unsigned int target, vector< string > recorded_path, vector< vector < string > > *total_path);
// added function for hw3
string retreive_path_name(string path);

int SetupOption(int argc, char ** argv)
{
    option.usage("[options] input_circuit_file");
    option.enroll("help", GetLongOpt::NoValue,
            "print this help summary", 0);
    option.enroll("logicsim", GetLongOpt::NoValue,
            "run logic simulation", 0);
    option.enroll("plogicsim", GetLongOpt::NoValue,
            "run parallel logic simulation", 0);
    option.enroll("fsim", GetLongOpt::NoValue,
            "run stuck-at fault simulation", 0);
    option.enroll("stfsim", GetLongOpt::NoValue,
            "run single pattern single transition-fault simulation", 0);
    option.enroll("transition", GetLongOpt::NoValue,
            "run transition-fault ATPG", 0);
    option.enroll("input", GetLongOpt::MandatoryValue,
            "set the input pattern file", 0);
    option.enroll("output", GetLongOpt::MandatoryValue,
            "set the output pattern file", 0);
    option.enroll("bt", GetLongOpt::OptionalValue,
            "set the backtrack limit", 0);
    // Modifier part
    option.enroll("pattern", GetLongOpt::NoValue,
            "set to generate random pattern", 0);
    option.enroll("num", GetLongOpt::MandatoryValue,
            "set number of gererated patterns", 0);
    option.enroll("ass0", GetLongOpt::NoValue,
            "count the required", 0);
    option.enroll("path", GetLongOpt::NoValue,
            "set to count the path numbers", 0);
    option.enroll("start", GetLongOpt::MandatoryValue,
            "set the start gate", 0);
    option.enroll("end", GetLongOpt::MandatoryValue,
            "set the end gate", 0);
    option.enroll("unknown", GetLongOpt::NoValue,
            "unknown involves or not", 0);
    option.enroll("mod_logicsim", GetLongOpt::NoValue,
            "modified version of simulation", 0);
    option.enroll("simulator", GetLongOpt::MandatoryValue,
            "generate simulation code", 0);
    option.enroll("check_point", GetLongOpt::NoValue,
            "do the fault generation with checkpoint theorem", 0);
    option.enroll("bridging", GetLongOpt::NoValue,
            "bridging fault simulation", 0);
    option.enroll("bridging_fsim", GetLongOpt::NoValue,
            "bridging fault simulation", 0);
    // end of modified part
    int optind = option.parse(argc, argv);
    if ( optind < 1 ) { exit(0); }
    if ( option.retrieve("help") ) {
        option.usage();
        exit(0);
    }
    return optind;
}

int main(int argc, char ** argv)
{
    int pid=(int) getpid();
    char buf[1024];
    sprintf(buf, "cat /proc/%d/statm",pid);
    system(buf);
    int optind = SetupOption(argc, argv);
    clock_t time_init, time_end;
    time_init = clock();
    //Setup File
    if (optind < argc) {
        if ((yyin = fopen(argv[optind], "r")) == NULL) {
            cout << "Can't open circuit file: " << argv[optind] << endl;
            exit( -1);
        }
        else {
            string circuit_name = argv[optind];
            string::size_type idx = circuit_name.rfind('/');
            if (idx != string::npos) { circuit_name = circuit_name.substr(idx+1); }
            idx = circuit_name.find(".bench");
            if (idx != string::npos) { circuit_name = circuit_name.substr(0,idx); }
            Circuit.SetName(circuit_name);
        }
    }
    else {
        cout << "Input circuit file missing" << endl;
        option.usage();
        return -1;
    }
    cout << "Start parsing input file\n";
    yyparse();
    if (ParseError) {
        cerr << "Please correct error and try Again.\n";
        return -1;
    }
    fclose(yyin);
    Circuit.FanoutList();
    Circuit.SetupIO_ID();
    Circuit.Levelize();
    Circuit.Check_Levelization();
    Circuit.InitializeQueue();
    

    if (option.retrieve("logicsim")) {
        //logic simulator
        //cout << "LOGIC SIM" << endl;
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.LogicSimVectors();
        cout << "evaluation numbers = " << Circuit.evaluate_num << endl;
    }
    else if (option.retrieve("plogicsim")) {
        //parallel logic simulator
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.ParallelLogicSimVectors();
    }
    else if (option.retrieve("stfsim")) {
        //single pattern single transition-fault simulation
        Circuit.MarkOutputGate();
        Circuit.GenerateAllTFaultList();
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.TFaultSimVectors();
    }
    else if (option.retrieve("transition")) {
        Circuit.MarkOutputGate();
        Circuit.GenerateAllTFaultList();
        Circuit.SortFaninByLevel();
        if (option.retrieve("bt")) {
            Circuit.SetBackTrackLimit(atoi(option.retrieve("bt")));
        }
        Circuit.TFAtpg();
    }
    // Modified part for hw0
    else if (option.retrieve("ass0")){
        cout << "Number of inputs: " << Circuit.No_PI() << endl;
        cout << "Number of outputs: " << Circuit.No_PO() << endl;
        // finding the number of gate types
        int No_gate=0;
        vector<unsigned int> showed_gate_type;
        unsigned int type[5] = {4, 5, 6, 7, 8}; // not, and, nand, or, nor
        unsigned int gate_numbers[12] = {0};
        for(unsigned int i=0; i<Circuit.No_Gate(); i++){
            unsigned int unconfirmed_type = Circuit.Gate(i)->GetFunction();
            //cout << unconfirmed_type << endl;
            // exception for input output and buffer gate
            if(unconfirmed_type == type[0] || unconfirmed_type == type[1] || unconfirmed_type == type[2] || unconfirmed_type == type[3] || unconfirmed_type == type[4]) No_gate++;
            gate_numbers[unconfirmed_type]++;
        }        
        string gate_names[12] = {"input gate", "output gate", "pseudo primary input gate", "pseudo primary output gate", "not gate", "and gate", "nand gate", "or gate", "nor gate", "flip flop", "buffer", "bad gate"};
        for(unsigned int i=0; i<12; i++){
            if(i == 0 || i == 1 || i == 9) continue;
            cout << "Number of " << gate_names[i] << ": " << gate_numbers[i] << endl;
        }
        cout << "Number of gates(including not, nand, and, nor, or): " << No_gate << endl;
        //cout << "Number of gates: " << Circuit.No_Gate() << endl;
        cout << "Number of flip-flops: " << Circuit.No_PPI() << endl;
        // finding the number of signals, branch nets , stem nets, and avg fanouts
        unsigned int No_signal=0, No_branchnet=0, No_stemnet=0;
        float avgfanout=0;
        for(unsigned int i=0; i<Circuit.No_Gate(); i++){
            
            //cout << "Function: " << Circuit.Gate(i)->GetFunction() << " Fanin: "<< Circuit.Gate(i)->No_Fanin() << " Fanout: "<< Circuit.Gate(i)->No_Fanout() << endl;
            avgfanout    += Circuit.Gate(i)->No_Fanout();
            No_branchnet += Circuit.Gate(i)->No_Fanin();
            No_stemnet   += Circuit.Gate(i)->No_Fanout();
            No_signal    += Circuit.Gate(i)->No_Fanin();
            No_signal    += Circuit.Gate(i)->No_Fanout();
            
            //cout << "i: " << i << " output-numbers: " << Circuit.Gate(i)->No_Fanout() << " input-numbers: " << Circuit.Gate(i)->No_Fanin() << " Function: " << Circuit.Gate(i)->GetFunction() << endl;
        }
        avgfanout /= Circuit.No_Gate();
        cout << "Number of signals: " << No_signal << endl;
        cout << "Number of branch nets: " << No_branchnet << endl;
        cout << "Number of stem nets: " << No_stemnet << endl;
        cout << "Average number of fanouts : " << avgfanout << endl;
    }
    // end of modified part

    // Modified part for hw1
    else if(option.retrieve("path")){
        string start_gate = option.retrieve("start");
        string end_gate = option.retrieve("end");
        unsigned int start_id;
        unsigned int end_id;
        for(unsigned int i=0; i<Circuit.No_Gate(); i++){
            string name_i = Circuit.Gate(i)->GetName();
            if(start_gate == name_i){
                //cout << "matched" << endl;
                //cout << i << endl;
                start_id = i;
            }
            if(end_gate == name_i){
                //cout << "matched" << endl;
                end_id = i;
            }
            //cout << Circuit.Gate(i)->GetName() << endl;
        }

        if(start_id == NULL){
            cout << "input gate doesn't exist!!" << endl;
            return 1;
        }

        if(end_id == NULL){
            cout << "output gate doesn't exist!!" << endl;
            return 1;
        }

        vector< string > init_path;
        vector< vector < string > > total_path;
        find_path(end_id, start_id, init_path, &total_path);
        //cout << Circuit.Gate(end_id)->GetInput_list()[0]->GetID() << endl;
        //cout << total_path.size();
        for(unsigned int i = 0; i < total_path.size(); i++){
            for(int j = total_path[i].size()-1; j >= 0 ; j--){
                //cout << j << endl;
                cout << total_path[i][j] << " ";
            }
            cout << endl;
        }
        cout << "The path from " << start_gate << " to " << end_gate << ": " << total_path.size() << endl;
    }
    // end modified
    // ass2
    else if (option.retrieve("pattern")){
        int num = stoi(option.retrieve("num"));
        // cout << num << endl;
        ofstream pattern;
        string output_name = option.retrieve("output");
        pattern.open(output_name);
        for (int i=0; i< Circuit.No_PI(); i++){
            pattern << "PI " << Circuit.PIGate(i)->GetName() << " ";
        }
        pattern << endl;

        srand(time(0));
        if(option.retrieve("unknown")){
            for (int i=0; i< num; i++){
                for (int j=0; j< Circuit.No_PI(); j++) {
                    int tmp = int(3.0*(rand() / (RAND_MAX + 1.0)));
                    if (tmp == 2) pattern << 'X';
                    else pattern << tmp;
                }
                pattern << endl;
            }
        }
        else{
            for (int i=0; i< num; i++){
                for (int j=0; j< Circuit.No_PI(); j++) pattern << int(2.0*(rand() / (RAND_MAX + 1.0)));
                pattern << endl;
            }
        }        
        pattern.close();
    }
    else if (option.retrieve("mod_logicsim")){
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.ModLogicSimVectors();
    }
    //end modified
    //ass3
    else if (option.retrieve("simulator")){
        string benchfile_path = argv[optind];
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.ParInitPrint(option.retrieve("simulator"), benchfile_path);        
    }
    else if (option.retrieve("check_point")){
        int all_fault=0;
        int checkpoint_fault=0;
        for (unsigned int i=0; i < Circuit.No_Gate(); i++){
            all_fault += Circuit.Gate(i)->No_Fanin();
            all_fault += Circuit.Gate(i)->No_Fanout();

            if (Circuit.Gate(i)->No_Fanout()>1){
                // branch
                checkpoint_fault = checkpoint_fault + Circuit.Gate(i)->No_Fanout();
            }
            if (Circuit.Gate(i)->GetFunction() == G_PI){
                // Primary Input
                checkpoint_fault += 1;
            }
        }

        all_fault *= 2;
        checkpoint_fault *= 2;

        cout << "number of total faults: " << all_fault << endl;
        cout << "number of collapsed fault: " << checkpoint_fault << endl;
        string tmp;
        float percentage = (float(all_fault - checkpoint_fault) / all_fault);
        percentage = percentage * 100;
        tmp = tmp + to_string(percentage) + " percentage of faults have been collapsed";
        cout << tmp << endl;
    }
    //ass4
    else if(option.retrieve("bridging")){
        Circuit.InitPattern(option.retrieve("input"));
        vector<string> store_list;
        for(int i=0; i<Circuit.GetMaxLevel(); i++){
            int prev_net=-1;
            for(int j=0; j<Circuit.No_Gate(); j++){
                if(Circuit.Gate(j)->GetLevel() == i){
                    if(prev_net != -1){
                        string tmp;
                        tmp = tmp + '(';
                        tmp = tmp + Circuit.Gate(prev_net)->GetName();
                        tmp = tmp + ", ";
                        tmp = tmp + Circuit.Gate(j)->GetName();
                        tmp = tmp + ", AND)\n";
                        tmp = tmp + '(';
                        tmp = tmp + Circuit.Gate(prev_net)->GetName();
                        tmp = tmp + ", ";
                        tmp = tmp + Circuit.Gate(j)->GetName();
                        tmp = tmp + ", OR)\n";
                        store_list.push_back(tmp);
                    }
                    prev_net = j;
                }
            }
        }
    }
    // ass 5
    else if(option.retrieve("bridging_fsim")){        
        Circuit.GenerateAllBridgeFaultList();
        Circuit.SortFaninByLevel();
        Circuit.MarkOutputGate();
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.BridgeFaultSimVectors();
    }
    else {
        Circuit.GenerateAllFaultList();
        Circuit.SortFaninByLevel();
        Circuit.MarkOutputGate();
        if (option.retrieve("fsim")) {
            //stuck-at fault simulator
            Circuit.InitPattern(option.retrieve("input"));
            Circuit.FaultSimVectors();
        }

        else {
            if (option.retrieve("bt")) {
                Circuit.SetBackTrackLimit(atoi(option.retrieve("bt")));
            }
            //stuck-at fualt ATPG
            Circuit.Atpg();
        }
    }
    time_end = clock();
    cout << "total CPU time = " << double(time_end - time_init)/CLOCKS_PER_SEC << endl;
    system("ps aux | grep atpg");
    cout << endl;
    return 0;
}

// Modified part for hw1
void find_path(unsigned int cur_ID, unsigned int target, vector< string > recorded_path, vector< vector < string > > *total_path){
    // If find a path
    //cout << "testing" << endl;
    //cout << cur_ID << endl;
    //cout << total_path.size() << endl;
    //for(int i = 0; i < recorded_path.size(); i++) cout << recorded_path[i] << " ";
    //cout << endl;
    if(cur_ID == target){
        recorded_path.push_back(Circuit.Gate(cur_ID)->GetName());
        //cout << "find" << endl;
        //for(int i = 0; i < recorded_path.size(); i++) cout << recorded_path[i] << " ";
        //cout << endl;
        total_path->push_back(recorded_path);
        //cout << total_path->size() << endl;
    }
    else{
        vector< string > new_path = recorded_path;
        new_path.push_back(Circuit.Gate(cur_ID)->GetName());
        for(unsigned int i=0; i < Circuit.Gate(cur_ID)->GetInput_list().size(); i++){
            unsigned int new_id = Circuit.Gate(cur_ID)->GetInput_list()[i]->GetID();
            find_path(new_id, target, new_path, total_path);
        }
    }
}

