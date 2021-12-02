#ifndef CFLINTHEART_HPP
#define CFLINTHEART_HPP

#include "actions/action.h"

#include "robots/robot.h"
#include "robots/builder.h"
#include "robots/creator.h"
#include "robots/transporter.h"

#include "resources/diamond.h"
#include "resources/gold.h"
#include "resources/metal.h"
#include "resources/oil.h"

#include "jinkou/CChuck.hpp"

// cena:
// ropa    - 225  *0.2
// metal   - 600  *0.5
// diamond - 200  *0.8
// złoto   - 200  *1.0
// ładowność:
// 400
// zdolności specjalne:
// jednak twardszy, niż diament

namespace jinkou {
    
    class CFlintheart: 
        virtual public robots::builder,
        virtual public CChuck{
        CZaiko *built_place;
        bool shouldIBuild;
        bool goSuperScout;
    protected:
        actions::action* build(const std::vector<actions::action *> &possibleactions);
        void changeMode();
        void autoPilot();
    public:
        actions::action* chooseAction(const std::vector<actions::action *> &possibleactions);
        CFlintheart(std::pair<int,int> in_c, CZaiko *in_bp):
                        CMyRobot(in_c),CMyMiner(in_c),CChuck(in_c),
                        built_place(in_bp),
                        shouldIBuild(false),goSuperScout(false)
                        {}
    };
}

#endif
