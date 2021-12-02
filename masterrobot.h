#ifndef MASTERROBOT_H
#define MASTERROBOT_H

#include<vector>
#include<bitset>

#include "logic/game.h"

#include "actions/action.h"

#include "robots/robot.h"
#include "robots/builder.h"
#include "robots/creator.h"
#include "robots/concreteminer.h"
#include "robots/transporter.h"

#include "resources/diamond.h"
#include "resources/gold.h"
#include "resources/metal.h"
#include "resources/oil.h"


#include "jinkou/Structures.hpp"
#include "jinkou/CScrooge.hpp"
#include "jinkou/CBob.hpp"
#include "jinkou/CFlintheart.hpp"
#include "jinkou/Shikikan.hpp"

// cena:
// ropa    - 450  *0.2
// metal   - 800  *0.5
// diamond - 400  *0.8
// złoto   -1000  *1.0
// surowce startowe:
// ropa    -  25  *0.2
// metal   - 100  *0.5
// diamond -   0  *0.8
// złoto   -  75  *1.0
// ładowność:
// 400
// power:
// OVER 9000!!!
// importance:
// CRUCIAL!
// informacje dodatkowe:
// jest nerwowy

namespace jinkou {
    
    __attribute__((constructor(1001)))
    void init_masterrobot();
    
    class masterrobot:
        // z pozdrowieniami dla autora tej gry:
        // wszystko wydziedziczone z naszych klas:
        virtual public CBob,
        virtual public CScrooge, // so he is Taichou!
        virtual public CFlintheart,
        virtual public CShougun{
        std::bitset<10> flags;
        /*
        meaning:
        0 - know he is Taichou
        1 - first site scouted
        2 - first plan done
        3 - metal not found
        4 - diamond not found
        5 - Bob built
        6 - Bob ordered
        7 - Flintheart built
        8 - direction change determiner, true at the beginning;
        */
        int starting_offset;
        void remember();
        void changeMode();
        void manageTeam(); // used ONLY in early phases;
        std::pair<int,int> searchSurroundings(char co);
        actions::action* lookForResources(const std::vector<actions::action*> &possibleactions, int ss=SITE_SIZE);
        actions::action* goRound(const std::vector<actions::action*> &possibleactions);
        actions::action* claimTheLand(const std::vector<actions::action *> &possibleactions);
        actions::action* earlyGame(const std::vector<actions::action*> &possibleactions);
        actions::action* earlyMidGame(const std::vector<actions::action*> &possibleactions);
        actions::action* midGame(const std::vector<actions::action*> &possibleactions);
    public:
        virtual actions::action *chooseAction(const std::vector<actions::action *> &possibleactions);
        masterrobot():
            CMyRobot(std::pair<int, int>(0, 0)),
            CMyMiner(std::pair<int, int>(0, 0)),
            CGajeel(std::pair<int,int>(0,0)),
            CBob(std::pair<int, int>(0, 0)),
            CScrooge(std::pair<int, int>(0, 0),-1,NULL,NULL), // ten -1 to tak dla pewności, coby ustalić, jaki jest jego dir;
            CChuck(std::pair<int,int>(0,0)),
            CFlintheart(std::pair<int, int>(0, 0),NULL),
            starting_offset(SITE_SIZE){
                flags.reset(); // even if redundant, executed to ensure all of them are zeros;
                flags.set(8);
            };
    };
        
    
}

#endif