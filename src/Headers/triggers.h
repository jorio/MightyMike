//
// Triggers.h
//

enum
{
	TRIGTYPE_TELEPORT,
	TRIGTYPE_DOOR,
	TRIGTYPE_FAIRYDOOR,
	TRIGTYPE_BARGAINDOOR
};


#define	TriggerSides	Kind
#define	TriggerType		Flag3

Boolean	HandleTrigger(ObjNode *, Byte);
Boolean	AddTeleport(ObjectEntryType *);
Boolean	DoTrig_Teleport(void);
Boolean	AddClowndoor(ObjectEntryType *);
Boolean	AddCandyDoor(ObjectEntryType *);
Boolean	AddJurassicDoor(ObjectEntryType *);
Boolean	AddBargainDoor(ObjectEntryType *);
void	MoveBargainDoor(void);
Boolean	AddFairyDoor(ObjectEntryType *);
void	MoveFairyDoor(void);
Boolean	DoTrig_FairyDoor(void);
Boolean	DoTrig_Door(void);
Boolean	DoTrig_BargainDoor(void);
