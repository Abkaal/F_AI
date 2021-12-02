#ifndef CCHUCK_HPP
#define CCHUCK_HPP

#include "actions/action.h"

#include "robots/robot.h"
#include "robots/builder.h"
#include "robots/creator.h"
#include "robots/transporter.h"

#include "resources/diamond.h"
#include "resources/gold.h"
#include "resources/metal.h"
#include "resources/oil.h"

#include "jinkou/CMyMiner.hpp"

// cena:
// ropa    - 225  *0.2
// metal   - 400  *0.5
// diamond - 200  *0.8
// złoto   - 200  *1.0
// ładowność:
// 400
// zdolności specjalne:
// tak naprawdę, to on wykopuje węgiel ( ͡° ͜ʖ ͡°)

namespace jinkou {
    
    class CChuck: 
        virtual public robots::concreteminer<resources::diamond>,
        virtual public CMyMiner{
    public:
        actions::action* chooseAction(const std::vector<actions::action *> &possibleactions);
        CChuck(std::pair<int,int> in_c):CMyRobot(in_c),CMyMiner(in_c){}
    };
    
}

#endif
