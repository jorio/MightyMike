//
// store.h
//



struct StoreDefType
{
	short		type;
	short		price;
	short		misc;
};
typedef struct StoreDefType StoreDefType;



extern void	EnterAStore(void);
extern void	DrawStoreImage(void);
extern void	DisplayStoreBuffer(void);
extern void	InitStoreCursor(void);
extern void	DrawStoreItems(void);
extern void	ProcessStore(void);
extern void	MoveStoreCursor(void);
extern void	EraseStore(void);
