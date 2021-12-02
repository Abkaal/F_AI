#ifndef CBOB_HPP
#define CBOB_HPP

#include "actions/action.h"

#include "robots/robot.h"
#include "robots/builder.h"
#include "robots/creator.h"
#include "robots/transporter.h"

#include "resources/diamond.h"
#include "resources/gold.h"
#include "resources/metal.h"
#include "resources/oil.h"

#include "jinkou/CGajeel.hpp"

// cena:
// ropa    - 225  *0.2
// metal   - 600  *0.5
// diamond - 200  *0.8
// złoto   - 200  *1.0
// ładowność:
// 400
// informacje dodatkowe:
// To jest Bob. Bob Budowniczy wszystko spierniczy, więc to Scrooge jest Taichou

namespace jinkou {
    
    class CBob: 
        virtual public robots::creator,
        virtual public CGajeel{
        bool swapped=false;
        TBrigade *new_brigade;
        std::vector<ERobot> *make_them;
        actions::action* create(const std::vector<actions::action *> &possibleactions, int ott=50);
        void changeMode();
    protected:
        void assignToBrigade(TBrigade* b, robots::robot* assignee, ERobot er);
        void autoPilot();
    public:
        void takeOrders(TOrders *orders, CTaichou *requester);
        virtual actions::action *chooseAction(const std::vector<actions::action *> &possibleactions);
        CBob(std::pair<int, int> coords, TBrigade *my_brigade=NULL, std::vector<ERobot> *in_mv=NULL):
                    CMyRobot(coords), CMyMiner(coords),
                    CGajeel(coords),
                    new_brigade(my_brigade),
                    make_them(in_mv){};
        virtual ~CBob(){
            if(new_brigade) delete new_brigade; new_brigade=NULL;
        }
    };
    
}

#endif
