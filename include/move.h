#pragma once

#include <string>

#include "types.h"

struct Move {
    int fr,fc,tr,tc;
    PieceType promo;
    bool castle,ep;

    Move():fr(0),fc(0),tr(0),tc(0),promo(EMPTY_P),castle(false),ep(false){}
    Move(int fr,int fc,int tr,int tc,
         PieceType p=EMPTY_P,bool c=false,bool e=false)
        :fr(fr),fc(fc),tr(tr),tc(tc),promo(p),castle(c),ep(e){}

    bool isResign() const { return fr==-1; }

    std::string str() const {
        if(isResign()) return "resign";
        std::string s;
        s+=(char)('a'+fc); s+=(char)('1'+fr);
        s+=(char)('a'+tc); s+=(char)('1'+tr);
        if(promo){ const char* pp=".pnbrqk"; s+=pp[promo]; }
        return s;
    }
    static Move resign(){ Move m; m.fr=m.fc=m.tr=m.tc=-1; return m; }
};
