//
// ITEM TABLE
//

#include "playfield.h"
#include "enemy.h"
#include "traps.h"
#include "triggers.h"
#include "bonus.h"

Boolean	(*gItemAddPtrs[])(ObjectEntryType *) = {
					AddEnemy1,
					AddAppearZone,
					AddStore,
					AddBunny
					};
							// REMEMBER TO REMOVE THE "IF > 3" FROM PLAYFIELD!!!!!!! ----


