#ifndef CSCROOGE_HPP
#define CSCROOGE_HPP

#include "logic/game.h"

#include "actions/action.h"

#include "robots/robot.h"
#include "robots/concreteminer.h"
#include "robots/transporter.h"

#include "resources/diamond.h"
#include "resources/gold.h"
#include "resources/metal.h"
#include "resources/oil.h"

#include "jinkou/Structures.hpp"
#include "jinkou/CMyMiner.hpp"
#include "jinkou/Shikikan.hpp"

// cena:
// ropa    - 450  *0.2
// metal   - 400  *0.5
// diamond - 0    *0.8
// złoto   - 600  *1.0
// ładowność:
// 400
// doświadczenie zawodowe:
// Więzień Doliny Białej Śmierci

namespace jinkou {
    
    class CScrooge: 
        virtual public robots::concreteminer<resources::gold>,
        virtual public robots::concreteminer<resources::oil>,
        virtual public robots::transporter,
        virtual public CMyMiner,
        virtual public CTaichou{
    public:
        virtual actions::action* chooseAction(const std::vector<actions::action *> &possibleactions);
        CScrooge(std::pair<int, int> coords, int in_d, TBrigade *in_b, TSite *in_s):CMyRobot(coords), CMyMiner(coords), CTaichou(in_d, in_b, in_s){};
        //CScrooge(std::pair<int, int> coords, TBrigade *in_b):CMyRobot(coords), CMyMiner(coords){};
        virtual ~CScrooge(){};
    };
        
    
}

#endif
