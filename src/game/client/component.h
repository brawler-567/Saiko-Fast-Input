 
 
#ifndef GAME_CLIENT_COMPONENT_H
#define GAME_CLIENT_COMPONENT_H

#if defined(CONF_VIDEORECORDER)
#include <engine/shared/video.h>
#endif

#include <engine/input.h>

class CGameClient;

class CComponentInterfaces
{
public:
	virtual void OnInterfacesInit(CGameClient *pClient);
	virtual ~CComponentInterfaces() = default;

protected:
	 
	class CGameClient *GameClient() const { return m_pClient; }

	 
	class IClient *Client() const;

	 
	class IKernel *Kernel() const;

	 
	class IEngine *Engine() const;

	 
	class IGraphics *Graphics() const;

	 
	class ITextRender *TextRender() const;

	 
	class IInput *Input() const;

	 
	class IStorage *Storage() const;

	 
	class CUi *Ui() const;

	 
	class ISound *Sound() const;

	 
	class CRenderTools *RenderTools() const;

	 
	class CRenderMap *RenderMap() const;

	 
	class IConfigManager *ConfigManager() const;

	 
	class CConfig *Config() const;

	 
	class IConsole *Console() const;

	 
	class IDemoPlayer *DemoPlayer() const;

	 
	class IDemoRecorder *DemoRecorder(int Recorder) const;

	 
	class IFavorites *Favorites() const;

	 
	class IServerBrowser *ServerBrowser() const;

	 
	class CLayers *Layers() const;

	 
	class CCollision *Collision() const;

#if defined(CONF_AUTOUPDATE)
	 
	class IUpdater *Updater() const;
#endif

	 
	int64_t time() const;

	 
	float LocalTime() const;

	 
	class IHttp *Http() const;

private:
	CGameClient *m_pClient = nullptr;
};

 
class CComponent : public CComponentInterfaces
{
public:
	 
	virtual int Sizeof() const = 0;

	 
	virtual void OnStateChange(int NewState, int OldState)
	{
	}

	 
	virtual void OnConsoleInit()
	{
	}

	 
	virtual void OnInit()
	{
	}

	 
	virtual void OnShutdown()
	{
	}

	 
	virtual void OnReset()
	{
	}

	 
	virtual void OnWindowResize()
	{
	}

	 
	virtual void OnUpdate()
	{
	}

	 
	virtual void OnRender()
	{
	}

	 
	virtual void OnNewSnapshot()
	{
	}

	 
	virtual void OnRelease()
	{
	}

	 
	virtual void OnMapLoad()
	{
	}

	 
	virtual void OnMessage(int Msg, void *pRawMsg)
	{
	}

	 
	virtual bool OnCursorMove(float x, float y, IInput::ECursorType CursorType)
	{
		return false;
	}

	 
	virtual bool OnInput(const IInput::CEvent &Event)
	{
		return false;
	}

	 
	virtual bool OnTouchState(const std::vector<IInput::CTouchFingerState> &vTouchFingerStates)
	{
		return false;
	}
};

#endif
