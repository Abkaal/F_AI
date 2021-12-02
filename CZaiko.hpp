#ifndef CZAIKO_HPP
#define CZAIKO_HPP

#include "exceptions/badformatexception.h"

#include "robots/warehouse.h"

#include "fields/resourcefield.h"

#include "Structures.hpp"

// cena:
// ropa    -   0  *0.2
// metal   - 100  *0.5
// diamond -   0  *0.8
// złoto   -  75  *1.0
// ładowność:
// 8LO
// zdolności:
// ...    ...   Nie do ruszenia!
// RICO, WYRZYGAJ COŚ PROSZĘ!!!

namespace jinkou{
    
    class CZaiko:virtual public robots::warehouse{
        std::pair<int, int> coords;
        std::pair<int, int> site_center;
        TSite *my_site;
        public:
        CZaiko() =delete;
        CZaiko(std::pair<int, int> in_c, TSite *in_site):
                    coords(in_c), my_site(in_site){
                        if(my_site->stockpile) throw new exceptions::badformatexception("WARNING!: REWAREHOUSING SITE!!!");
                        my_site->stockpile=this;
                    }
        std::pair<int, int> getCoords(){return coords;}
        std::pair<int, int> getSiteCenter(){return site_center;}
        TSite* getSite(){return my_site;}
    };
}

#endif