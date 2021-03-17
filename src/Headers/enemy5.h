//
// Enemy5.h
//

			/* BATTERY */

Boolean	AddEnemy_Battery(ObjectEntryType *);
void	MoveBattery(void);
short	GuessBatteryAnim(short, short);


			/* SLINKY */

Boolean	AddEnemy_Slinky(ObjectEntryType *);
void	MoveSlinky(void);
short	GuessSlinkyAnim(short, short, short);
void	CalcSlinkyBox(void);


			/* 8BALL */

Boolean	AddEnemy_8Ball(ObjectEntryType *);
void	Move8Ball(void);


			/* ROBOT */

Boolean	AddEnemy_Robot(ObjectEntryType *);
void	MoveRobot(void);
void	DoRobotMove(void);
void	DoRobotDanger(void);


			/* DOGGY */

Boolean	AddEnemy_Doggy(ObjectEntryType *);
void	MoveDoggy(void);
void	MoveDoggy_Walk(void);
void	MoveDoggy_Wag(void);
void	MoveDoggy_Jump(void);
void	UpdateDoggy(void);
void	DoDoggyMove(void);
void	DoDogRoar(void);

			/* TOP */

Boolean	AddEnemy_Top(ObjectEntryType *);
void	MoveTop(void);
void	UpdateTop(void);
void	DoTopMove(void);
