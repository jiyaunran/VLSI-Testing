#ifndef FAULT_H
#define FAULT_H
#include "gate.h"

class FAULT
{
    private:
        VALUE Value;
        GATE* Input;
        GATE* Output; //record output gate for branch fault
        //if stem, Input = Output
        bool Branch; //fault is on branch
        unsigned EqvFaultNum; //equivalent fault number (includes itself)
        FAULT_STATUS Status;
    public:
        FAULT(GATE* gptr, GATE* ogptr, VALUE value): Value(value), Input(gptr),
        Output(ogptr), Branch(false), EqvFaultNum(1), Status(UNKNOWN) {}
        ~FAULT() {}
        VALUE GetValue() { return Value; }
        GATE* GetInputGate() { return Input; }
        GATE* GetOutputGate() { return Output; }
        void SetBranch(bool b) { Branch = b; }
        bool Is_Branch() { return Branch; }
        void SetEqvFaultNum(unsigned n) { EqvFaultNum = n; }
        void IncEqvFaultNum() { ++EqvFaultNum; }
        unsigned GetEqvFaultNum() { return EqvFaultNum; }
        void SetStatus(FAULT_STATUS status) { Status = status; }
        FAULT_STATUS GetStatus() { return Status; }
};

class Bridge_FAULT
{
    private:
        VALUE Value;
        GATE* Input1;
        GATE* Input2;
        GATE* Output1; //record output gate for branch fault
        GATE* Output2; //record output gate for branch fault
        //if stem, Input = Output
        bool Branch; //fault is on branch
        unsigned EqvFaultNum; //equivalent fault number (includes itself)
        FAULT_STATUS Status;
    public:
        Bridge_FAULT(GATE* gptr1, GATE* gptr2, GATE* ogptr1, GATE* ogptr2, VALUE value): Value(value), Input1(gptr1), Input2(gptr2),
        Output1(ogptr1), Output2(ogptr2), Branch(false), EqvFaultNum(1), Status(UNKNOWN) {}
        ~Bridge_FAULT() {}
        VALUE GetValue() { return Value; }
        GATE* GetInputGate1() { return Input1; }
        GATE* GetInputGate2() { return Input2; }
        GATE* GetOutputGate1() { return Output1; }
        GATE* GetOutputGate2() { return Output2; }
        void SetBranch(bool b) { Branch = b; }
        bool Is_Branch() { return Branch; }
        void SetEqvFaultNum(unsigned n) { EqvFaultNum = n; }
        void IncEqvFaultNum() { ++EqvFaultNum; }
        unsigned GetEqvFaultNum() { return EqvFaultNum; }
        void SetStatus(FAULT_STATUS status) { Status = status; }
        FAULT_STATUS GetStatus() { return Status; }
};



#endif
