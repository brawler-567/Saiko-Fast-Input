 
 
#ifndef GAME_CLIENT_COMPONENTS_SKINS_H
#define GAME_CLIENT_COMPONENTS_SKINS_H

#include <base/lock.h>

#include <engine/shared/config.h>
#include <engine/shared/jobs.h>

#include <game/client/component.h>
#include <game/client/skin.h>

#include <chrono>
#include <list>
#include <optional>
#include <set>
#include <string_view>
#include <unordered_map>
#include <utility>

class CHttpRequest;

class CSkins : public CComponent
{
private:
	 
	class CSkinLoadData
	{
	public:
		CImageInfo m_Info;
		CImageInfo m_InfoGrayscale;
		CSkin::CSkinMetrics m_Metrics;
		ColorRGBA m_BloodColor;
	};

	 
	class CAbstractSkinLoadJob : public IJob
	{
	public:
		CAbstractSkinLoadJob(CSkins *pSkins, const char *pName);
		~CAbstractSkinLoadJob() override;

		CSkinLoadData m_Data;
		bool m_NotFound = false;

	protected:
		CSkins *m_pSkins;
		char m_aName[MAX_SKIN_LENGTH];
	};

public:
	 
	class CSkinContainer
	{
		friend class CSkins;

	public:
		enum class EType
		{
			 
			LOCAL,
			 
			DOWNLOAD,
		};

		enum class EState
		{
			 
			UNLOADED,
			 
			PENDING,
			 
			LOADING,
			 
			LOADED,
			 
			ERROR,
			 
			NOT_FOUND,
		};

		CSkinContainer(CSkinContainer &&Other) = default;
		CSkinContainer(CSkins *pSkins, const char *pName, EType Type, int StorageType);
		~CSkinContainer();

		bool operator<(const CSkinContainer &Other) const;
		CSkinContainer &operator=(CSkinContainer &&Other) = default;

		const char *Name() const { return m_aName; }
		EType Type() const { return m_Type; }
		int StorageType() const { return m_StorageType; }
		bool IsVanilla() const { return m_Vanilla; }
		bool IsSpecial() const { return m_Special; }
		bool IsAlwaysLoaded() const { return m_AlwaysLoaded; }
		EState State() const { return m_State; }
		const std::unique_ptr<CSkin> &Skin() const { return m_pSkin; }

		 
		void RequestLoad();

	private:
		CSkins *m_pSkins;
		char m_aName[MAX_SKIN_LENGTH];
		EType m_Type;
		int m_StorageType;
		bool m_Vanilla;
		bool m_Special;
		bool m_AlwaysLoaded;

		EState m_State = EState::UNLOADED;
		std::unique_ptr<CSkin> m_pSkin = nullptr;
		std::shared_ptr<CAbstractSkinLoadJob> m_pLoadJob = nullptr;

		 
		std::optional<std::chrono::nanoseconds> m_FirstLoadRequest;
		 
		std::optional<std::chrono::nanoseconds> m_LastLoadRequest;
		 
		std::optional<std::list<std::string_view>::iterator> m_UsageEntryIterator;

		EState DetermineInitialState() const;
		void SetState(EState State);
	};

	 
	class CSkinListEntry
	{
	public:
		CSkinListEntry() :
			m_pSkinContainer(nullptr),
			m_Favorite(false) {}
		CSkinListEntry(CSkinContainer *pSkinContainer, bool Favorite, bool SelectedMain, bool SelectedDummy, std::optional<std::pair<int, int>> NameMatch) :
			m_pSkinContainer(pSkinContainer),
			m_Favorite(Favorite),
			m_SelectedMain(SelectedMain),
			m_SelectedDummy(SelectedDummy),
			m_NameMatch(NameMatch) {}

		bool operator<(const CSkinListEntry &Other) const;

		const CSkinContainer *SkinContainer() const { return m_pSkinContainer; }
		bool IsFavorite() const { return m_Favorite; }
		bool IsSelectedMain() const { return m_SelectedMain; }
		bool IsSelectedDummy() const { return m_SelectedDummy; }
		const std::optional<std::pair<int, int>> &NameMatch() const { return m_NameMatch; }

		const void *ListItemId() const { return &m_ListItemId; }
		const void *FavoriteButtonId() const { return &m_FavoriteButtonId; }
		const void *ErrorTooltipId() const { return &m_ErrorTooltipId; }

		 
		void RequestLoad();

	private:
		CSkinContainer *m_pSkinContainer;
		bool m_Favorite;
		bool m_SelectedMain;
		bool m_SelectedDummy;
		std::optional<std::pair<int, int>> m_NameMatch;
		char m_ListItemId;
		char m_FavoriteButtonId;
		char m_ErrorTooltipId;
	};

	class CSkinList
	{
		friend class CSkins;

	public:
		std::vector<CSkinListEntry> &Skins() { return m_vSkins; }
		int UnfilteredCount() const { return m_UnfilteredCount; }
		void ForceRefresh() { m_NeedsUpdate = true; }

	private:
		std::vector<CSkinListEntry> m_vSkins;
		int m_UnfilteredCount;
		bool m_NeedsUpdate = true;
	};

	class CSkinLoadingStats
	{
	public:
		size_t m_NumUnloaded = 0;
		size_t m_NumPending = 0;
		size_t m_NumLoading = 0;
		size_t m_NumLoaded = 0;
		size_t m_NumError = 0;
		size_t m_NumNotFound = 0;
	};

	CSkins();

	typedef std::function<void()> TSkinLoadedCallback;

	int Sizeof() const override { return sizeof(*this); }
	void OnConsoleInit() override;
	void OnInit() override;
	void OnShutdown() override;
	void OnUpdate() override;

	void RefreshEventSkins();
	void Refresh(TSkinLoadedCallback &&SkinLoadedCallback);
	CSkinLoadingStats LoadingStats() const;
	CSkinList &SkinList();

	const CSkinContainer *FindContainerOrNullptr(const char *pName);
	const CSkin *FindOrNullptr(const char *pName);
	const CSkin *Find(const char *pName);

	void AddFavorite(const char *pName);
	void RemoveFavorite(const char *pName);
	bool IsFavorite(const char *pName) const;

	void RandomizeSkin(int Dummy);

	const char *SkinPrefix() const;

	static bool IsSpecialSkin(const char *pName);

private:
	static bool IsVanillaSkin(const char *pName);

	 
	constexpr static const char *VANILLA_SKINS[] = {"bluekitty", "bluestripe", "brownbear",
		"cammo", "cammostripes", "coala", "default", "limekitty",
		"pinky", "redbopp", "redstripe", "saddo", "toptri",
		"twinbop", "twintri", "warpaint", "x_ninja", "x_spec"};

	class CSkinLoadJob : public CAbstractSkinLoadJob
	{
	public:
		CSkinLoadJob(CSkins *pSkins, const char *pName, int StorageType);

	protected:
		void Run() override;

	private:
		int m_StorageType;
	};

	class CSkinDownloadJob : public CAbstractSkinLoadJob
	{
	public:
		CSkinDownloadJob(CSkins *pSkins, const char *pName);

		bool Abort() override REQUIRES(!m_Lock);

	protected:
		void Run() override REQUIRES(!m_Lock);

	private:
		CLock m_Lock;
		std::shared_ptr<CHttpRequest> m_pGetRequest GUARDED_BY(m_Lock);
	};

	std::unordered_map<std::string_view, std::unique_ptr<CSkinContainer>> m_Skins;
	std::optional<std::chrono::nanoseconds> m_ContainerUpdateTime;
	 
	std::list<std::string_view> m_SkinsUsageList;

	CSkinList m_SkinList;
	std::set<std::string> m_Favorites;

	CSkin m_PlaceholderSkin;
	char m_aEventSkinPrefix[MAX_SKIN_LENGTH];

	bool LoadSkinData(const char *pName, CSkinLoadData &Data) const;
	void LoadSkinFinish(CSkinContainer *pSkinContainer, const CSkinLoadData &Data);
	void LoadSkinDirect(const char *pName);
	const CSkinContainer *FindContainerImpl(const char *pName);
	static int SkinScan(const char *pName, int IsDir, int StorageType, void *pUser);

	void UpdateUnloadSkins(CSkinLoadingStats &Stats);
	void UpdateStartLoading(CSkinLoadingStats &Stats);
	void UpdateFinishLoading(CSkinLoadingStats &Stats, std::chrono::nanoseconds StartTime, std::chrono::nanoseconds MaxTime);

	static void ConAddFavoriteSkin(IConsole::IResult *pResult, void *pUserData);
	static void ConRemFavoriteSkin(IConsole::IResult *pResult, void *pUserData);
	static void ConfigSaveCallback(IConfigManager *pConfigManager, void *pUserData);
	void OnConfigSave(IConfigManager *pConfigManager);
	static void ConchainRefreshSkinList(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);

	friend class CSkinProfiles;
};
#endif
