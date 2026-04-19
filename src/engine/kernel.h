 
 
#ifndef ENGINE_KERNEL_H
#define ENGINE_KERNEL_H

class IKernel;
class IInterface;

class IInterface
{
	 
	friend class CKernel;
	IKernel *m_pKernel;

protected:
	IKernel *Kernel() { return m_pKernel; }

public:
	IInterface() :
		m_pKernel(nullptr) {}
	virtual void Shutdown() {}
	virtual ~IInterface() = default;
};

#define MACRO_INTERFACE(Name) \
public: \
	static const char *InterfaceName() { return Name; } \
\
private:

 
 
class IKernel
{
	 
	virtual void RegisterInterfaceImpl(const char *pInterfaceName, IInterface *pInterface, bool Destroy) = 0;
	virtual void ReregisterInterfaceImpl(const char *pInterfaceName, IInterface *pInterface) = 0;
	virtual IInterface *RequestInterfaceImpl(const char *pInterfaceName) = 0;

public:
	static IKernel *Create();
	virtual void Shutdown() = 0;
	virtual ~IKernel() = default;

	 
	template<class TINTERFACE>
	void RegisterInterface(TINTERFACE *pInterface, bool Destroy = true)
	{
		RegisterInterfaceImpl(TINTERFACE::InterfaceName(), pInterface, Destroy);
	}
	template<class TINTERFACE>
	void ReregisterInterface(TINTERFACE *pInterface)
	{
		ReregisterInterfaceImpl(TINTERFACE::InterfaceName(), pInterface);
	}

	 
	 
	template<class TINTERFACE>
	TINTERFACE *RequestInterface()
	{
		return reinterpret_cast<TINTERFACE *>(RequestInterfaceImpl(TINTERFACE::InterfaceName()));
	}
};

#endif
