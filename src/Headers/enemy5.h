//
// Enemy5.h
//

			/* BATTERY */

extern Boolean	AddEnemy_Battery(ObjectEntryType *);
extern void	MoveBattery(void);
extern short	GuessBatteryAnim(short, short);


			/* SLINKY */

extern Boolean	AddEnemy_Slinky(ObjectEntryType *);
extern void	MoveSlinky(void);
extern short	GuessSlinkyAnim(short, short, short);
extern void	CalcSlinkyBox(void);


			/* 8BALL */

extern Boolean	AddEnemy_8Ball(ObjectEntryType *);
extern void	Move8Ball(void);


			/* ROBOT */

extern Boolean	AddEnemy_Robot(ObjectEntryType *);
extern void	MoveRobot(void);
extern void	DoRobotMove(void);
extern void	DoRobotDanger(void);


			/* DOGGY */

extern Boolean	AddEnemy_Doggy(ObjectEntryType *);
extern void	MoveDoggy(void);
extern void	MoveDoggy_Walk(void);
extern void	MoveDoggy_Wag(void);
extern void	MoveDoggy_Jump(void);
extern void	UpdateDoggy(void);
extern void	DoDoggyMove(void);
extern void	DoDogRoar(void);

			/* TOP */

extern Boolean	AddEnemy_Top(ObjectEntryType *);
extern void	MoveTop(void);
extern void	UpdateTop(void);
extern void	DoTopMove(void);
