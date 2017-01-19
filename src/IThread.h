#ifndef ITHREAD_H
#define ITHREAD_H

class CCDAudio;

/**
*	Interface for the CD Audio thread.
*/
class IThread
{
public:
	using CallbackFn = void ( CCDAudio::* )( int, int );

public:
	virtual ~IThread() = 0;

	virtual bool Init() = 0;

	virtual void Shutdown() = 0;

	/**
	*	Adds a new item to the list.
	*	@param callback Callback to invoke
	*	@param param1 First parameter
	*	@param param2 Second parameter
	*	@return Whether the item was added to the list
	*/
	virtual bool AddThreadItem( CallbackFn callback, int param1, int param2 ) = 0;
};

inline IThread::~IThread()
{
}

#endif //ITHREAD_H
