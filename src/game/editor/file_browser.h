 
 
#ifndef GAME_EDITOR_FILE_BROWSER_H
#define GAME_EDITOR_FILE_BROWSER_H

#include "component.h"

#include <base/types.h>

#include <game/client/ui.h>
#include <game/client/ui_listbox.h>

#include <optional>
#include <vector>

class CFileBrowser : public CEditorComponent
{
public:
	enum class EFileType
	{
		MAP,
		IMAGE,
		SOUND,
	};
	typedef bool (*FFileDialogOpenCallback)(const char *pFilename, int StorageType, void *pUser);

	void ShowFileDialog(
		int StorageType, EFileType FileType,
		const char *pTitle, const char *pButtonText,
		const char *pInitialPath, const char *pInitialFilename,
		FFileDialogOpenCallback pfnOpenCallback, void *pOpenCallbackUser);
	void OnRender(CUIRect _) override;
	bool IsValidSaveFilename() const;

	void OnEditorClose();
	void OnDialogClose();

private:
	 
	int m_StorageType = 0;
	 
	EFileType m_FileType = EFileType::MAP;
	 
	bool m_SaveAction = false;
	 
	bool m_MultipleStorages = false;
	 
	char m_aTitle[128] = "";
	 
	char m_aButtonText[64] = "";
	 
	FFileDialogOpenCallback m_pfnOpenCallback = nullptr;
	 
	void *m_pOpenCallbackUser = nullptr;
	 
	bool m_ShowingRoot = false;
	 
	char m_aInitialFolder[IO_MAX_PATH_LENGTH] = "";
	 
	char m_aCurrentFolder[IO_MAX_PATH_LENGTH] = "";
	 
	char m_aCurrentLink[IO_MAX_PATH_LENGTH] = "";
	 
	char *m_pCurrentPath = m_aCurrentFolder;
	 
	CLineInputBuffered<IO_MAX_PATH_LENGTH> m_FilenameInput;
	 
	CLineInputBuffered<IO_MAX_PATH_LENGTH> m_FilterInput;
	 
	int m_SelectedFileIndex = -1;
	 
	char m_aSelectedFileDisplayName[IO_MAX_PATH_LENGTH] = "";

	 
	class CFilelistItem
	{
	public:
		char m_aFilename[IO_MAX_PATH_LENGTH];
		char m_aDisplayName[IO_MAX_PATH_LENGTH];
		bool m_IsDir;
		bool m_IsLink;
		int m_StorageType;
		time_t m_TimeModified;
	};
	std::vector<CFilelistItem> m_vCompleteFileList;
	std::vector<const CFilelistItem *> m_vpFilteredFileList;
	enum class ESortDirection
	{
		NEUTRAL,
		ASCENDING,
		DESCENDING,
	};
	ESortDirection m_SortByFilename = ESortDirection::ASCENDING;
	ESortDirection m_SortByTimeModified = ESortDirection::NEUTRAL;

	 
	enum class EPreviewState
	{
		UNLOADED,
		LOADED,
		ERROR,
	};
	EPreviewState m_PreviewState = EPreviewState::UNLOADED;
	IGraphics::CTextureHandle m_PreviewImage;
	int m_PreviewImageWidth = 0;
	int m_PreviewImageHeight = 0;
	int m_PreviewSound = -1;

	 
	CListBox m_ListBox;
	const char m_ButtonSortTimeModifiedId = 0;
	const char m_ButtonSortFilenameId = 0;
	const char m_ButtonPlayPauseId = 0;
	const char m_ButtonStopId = 0;
	const char m_SeekBarId = 0;
	const char m_ButtonOkId = 0;
	const char m_ButtonCancelId = 0;
	const char m_ButtonRefreshId = 0;
	const char m_ButtonShowDirectoryId = 0;
	const char m_ButtonDeleteId = 0;
	const char m_ButtonNewFolderId = 0;

	bool CanPreviewFile() const;
	void UpdateFilePreview();
	void RenderFilePreview(CUIRect Preview);
	const char *DetermineFileFontIcon(const CFilelistItem *pItem) const;
	void UpdateFilenameInput();
	void UpdateSelectedIndex(const char *pDisplayName);
	void SortFilteredFileList();
	void RefreshFilteredFileList();
	void FilelistPopulate(int StorageType, bool KeepSelection);
	static int DirectoryListingCallback(const CFsFileInfo *pInfo, int IsDir, int StorageType, void *pUser);
	static std::optional<bool> CompareCommon(const CFilelistItem *pLhs, const CFilelistItem *pRhs);
	static bool CompareFilenameAscending(const CFilelistItem *pLhs, const CFilelistItem *pRhs);
	static bool CompareFilenameDescending(const CFilelistItem *pLhs, const CFilelistItem *pRhs);
	static bool CompareTimeModifiedAscending(const CFilelistItem *pLhs, const CFilelistItem *pRhs);
	static bool CompareTimeModifiedDescending(const CFilelistItem *pLhs, const CFilelistItem *pRhs);

	class CPopupNewFolder : public SPopupMenuId
	{
	public:
		CFileBrowser *m_pFileBrowser;
		CLineInputBuffered<IO_MAX_PATH_LENGTH> m_NewFolderNameInput;
		static CUi::EPopupMenuFunctionResult Render(void *pContext, CUIRect View, bool Active);

	private:
		const char m_ButtonCancelId = 0;
		const char m_ButtonCreateId = 0;
	};
	CPopupNewFolder m_PopupNewFolder;

	class CPopupConfirmDelete : public SPopupMenuId
	{
	public:
		CFileBrowser *m_pFileBrowser;
		bool m_IsDirectory;
		char m_aDeletePath[IO_MAX_PATH_LENGTH];
		static CUi::EPopupMenuFunctionResult Render(void *pContext, CUIRect View, bool Active);

	private:
		const char m_ButtonCancelId = 0;
		const char m_ButtonDeleteId = 0;
	};
	CPopupConfirmDelete m_PopupConfirmDelete;

	class CPopupConfirmOverwrite : public SPopupMenuId
	{
	public:
		CFileBrowser *m_pFileBrowser;
		char m_aOverwritePath[IO_MAX_PATH_LENGTH];
		static CUi::EPopupMenuFunctionResult Render(void *pContext, CUIRect View, bool Active);

	private:
		const char m_ButtonCancelId = 0;
		const char m_ButtonOverwriteId = 0;
	};
	CPopupConfirmOverwrite m_PopupConfirmOverwrite;
};

#endif
