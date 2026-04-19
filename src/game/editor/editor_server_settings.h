#ifndef GAME_EDITOR_EDITOR_SERVER_SETTINGS_H
#define GAME_EDITOR_EDITOR_SERVER_SETTINGS_H

#include "component.h"
#include "editor_ui.h"

#include <map>
#include <string>
#include <vector>

class CEditor;
struct SMapSettingInt;
struct SMapSettingCommand;
struct IMapSetting;
class CLineInput;

struct CEditorMapSetting
{
	char m_aCommand[256];

	CEditorMapSetting(const char *pCommand)
	{
		str_copy(m_aCommand, pCommand);
	}
};

 
 
struct SParsedMapSettingArg
{
	char m_aName[32];
	char m_Type;
};

 
struct SCurrentSettingArg
{
	char m_aValue[256];  
	float m_X;  
	size_t m_Start;  
	size_t m_End;  
	bool m_Error;  
	char m_ExpectedType;  
};

struct SPossibleValueMatch
{
	const char *m_pValue;  
	int m_ArgIndex;  
	const void *m_pData;  
};

struct SCommandParseError
{
	char m_aMessage[256];
	int m_ArgIndex;

	enum EErrorType
	{
		ERROR_NONE = 0,
		ERROR_TOO_MANY_ARGS,
		ERROR_INVALID_VALUE,
		ERROR_UNKNOWN_VALUE,
		ERROR_INCOMPLETE,
		ERROR_OUT_OF_RANGE,
		ERROR_UNKNOWN
	};
	EErrorType m_Type;
};

struct SInvalidSetting
{
	enum
	{
		TYPE_INVALID = 1 << 0,
		TYPE_DUPLICATE = 1 << 1
	};
	int m_Index;  
	char m_aSetting[256];  
	int m_Type;  
	int m_CollidingIndex;  
	bool m_Unknown;  

	struct SContext
	{
		bool m_Fixed;
		bool m_Deleted;
		bool m_Chosen;
	} m_Context;

	SInvalidSetting(int Index, const char *pSetting, int Type, int CollidingIndex, bool Unknown) :
		m_Index(Index), m_Type(Type), m_CollidingIndex(CollidingIndex), m_Unknown(Unknown), m_Context()
	{
		str_copy(m_aSetting, pSetting);
	}
};

 
 
 
 
 
 
 
 
 
 
 
 
 

using TArgumentValuesList = std::vector<const char *>;  
using TSettingValues = std::map<int, TArgumentValuesList>;  
using TSettingsArgumentValues = std::map<std::string, TSettingValues>;  

class CValuesBuilder;
class CSettingValuesBuilder;
class CArgumentValuesListBuilder
{
public:
	CArgumentValuesListBuilder &Add(const char *pString)
	{
		m_pContainer->emplace_back(pString);
		return *this;
	}

private:
	CArgumentValuesListBuilder(std::vector<const char *> *pContainer) :
		m_pContainer(pContainer) {}

	std::vector<const char *> *m_pContainer;
	friend class CSettingValuesBuilder;
};

class CSettingValuesBuilder
{
public:
	CArgumentValuesListBuilder Argument(int Arg) const
	{
		return CArgumentValuesListBuilder(&(*m_pContainer)[Arg]);
	}

private:
	CSettingValuesBuilder(TSettingValues *pContainer) :
		m_pContainer(pContainer) {}

	friend class CValuesBuilder;
	TSettingValues *m_pContainer;
};

class CValuesBuilder
{
public:
	CValuesBuilder(TSettingsArgumentValues *pContainer) :
		m_pContainer(pContainer)
	{
	}

	CSettingValuesBuilder operator()(const char *pSettingName) const
	{
		return CSettingValuesBuilder(&(*m_pContainer)[pSettingName]);
	}

private:
	TSettingsArgumentValues *m_pContainer;
};

 

struct SValueLoader
{
	static void LoadTuneValues(const CSettingValuesBuilder &TuneBuilder);
	static void LoadTuneZoneValues(const CSettingValuesBuilder &TuneZoneBuilder);
	static void LoadMapBugs(const CSettingValuesBuilder &BugBuilder);
	static void LoadArgumentTuneValues(CArgumentValuesListBuilder &&ArgBuilder);
};

enum class EValidationResult
{
	VALID = 0,
	ERROR,
	INCOMPLETE,
	UNKNOWN,
	OUT_OF_RANGE,
};

enum class ECollisionCheckResult
{
	ERROR,
	REPLACE,
	ADD
};

class CMapSettingsBackend : public CEditorComponent
{
	typedef void (*FLoaderFunction)(const CSettingValuesBuilder &);

public:
	 
	CMapSettingsBackend() = default;

	void OnInit(CEditor *pEditor) override;
	void OnMapLoad() override;

	 
	enum class EArgConstraint
	{
		DEFAULT = 0,
		UNIQUE,
		MULTIPLE,
	};

	EArgConstraint ArgConstraint(const char *pSettingName, int Arg) const
	{
		return m_ArgConstraintsPerCommand.at(pSettingName).at(Arg);
	}

	 
	const std::vector<SParsedMapSettingArg> &ParsedArgs(const std::shared_ptr<IMapSetting> &pSetting) const
	{
		return m_ParsedCommandArgs.at(pSetting);
	}

	 
	class CContext
	{
		static constexpr ColorRGBA ms_ArgumentStringColor = ColorRGBA(84.0f / 255.0f, 1.0f, 1.0f, 1.0f);
		static constexpr ColorRGBA ms_ArgumentNumberColor = ColorRGBA(0.1f, 0.9f, 0.05f, 1.0f);
		static constexpr ColorRGBA ms_ArgumentUnknownColor = ColorRGBA(0.6f, 0.6f, 0.6f, 1.0f);
		static constexpr ColorRGBA ms_CommentColor = ColorRGBA(0.5f, 0.5f, 0.5f, 1.0f);
		static constexpr ColorRGBA ms_ErrorColor = ColorRGBA(240.0f / 255.0f, 70.0f / 255.0f, 70.0f / 255.0f, 1.0f);

		friend class CMapSettingsBackend;

	public:
		bool CommandIsValid() const { return m_pCurrentSetting != nullptr; }
		int CurrentArg() const { return m_CursorArgIndex; }
		const char *CurrentArgName() const { return (!m_pCurrentSetting || m_CursorArgIndex < 0 || m_CursorArgIndex >= (int)m_pBackend->m_ParsedCommandArgs.at(m_pCurrentSetting).size()) ? nullptr : m_pBackend->m_ParsedCommandArgs.at(m_pCurrentSetting).at(m_CursorArgIndex).m_aName; }
		float CurrentArgPos() const { return (m_CursorArgIndex < 0 || m_CursorArgIndex >= (int)m_vCurrentArgs.size()) ? 0 : m_vCurrentArgs[m_CursorArgIndex].m_X; }
		size_t CurrentArgOffset() const { return m_CursorArgIndex == -1 ? 0 : m_vCurrentArgs[m_CursorArgIndex].m_Start; }
		const char *CurrentArgValue() const { return m_CursorArgIndex == -1 ? m_aCommand : m_vCurrentArgs[m_CursorArgIndex].m_aValue; }
		const std::vector<SPossibleValueMatch> &PossibleMatches() const { return m_vPossibleMatches; }
		bool HasError() const { return m_Error.m_aMessage[0] != '\0'; }
		size_t ErrorOffset() const { return m_Error.m_ArgIndex < 0 ? 0 : m_vCurrentArgs.at(m_Error.m_ArgIndex).m_Start; }
		const char *Error() const { return m_Error.m_aMessage; }
		int ArgCount() const { return (int)m_vCurrentArgs.size(); }
		const SCurrentSettingArg &Arg(int Index) const { return m_vCurrentArgs.at(Index); }
		const std::shared_ptr<IMapSetting> &Setting() const { return m_pCurrentSetting; }
		CLineInput *LineInput() const { return m_pLineInput; }
		void SetFontSize(float FontSize) { m_FontSize = FontSize; }
		int CommentOffset() const { return m_CommentOffset; }

		int CheckCollision(ECollisionCheckResult &Result) const;
		int CheckCollision(const std::vector<CEditorMapSetting> &vSettings, ECollisionCheckResult &Result) const;
		int CheckCollision(const char *pInputString, const std::vector<CEditorMapSetting> &vSettings, ECollisionCheckResult &Result) const;

		void Update();
		void UpdateFromString(const char *pStr);
		bool UpdateCursor(bool Force = false);
		void Reset();
		void GetCommandHelpText(char *pStr, int Length) const;
		bool Valid() const;
		void ColorArguments(std::vector<STextColorSplit> &vColorSplits) const;

		bool m_AllowUnknownCommands;
		SEditBoxDropdownContext m_DropdownContext;
		int m_CurrentCompletionIndex;

	private:
		 
		CContext(CMapSettingsBackend *pMaster, CLineInput *pLineinput) :
			m_DropdownContext(), m_pLineInput(pLineinput), m_pBackend(pMaster)
		{
			m_AllowUnknownCommands = false;
			Reset();
		}

		void ClearError();
		EValidationResult ValidateArg(int Index, const char *pArg);
		void UpdatePossibleMatches();
		void ParseArgs(const char *pLineInputStr, const char *pStr);
		bool OnInput(const IInput::CEvent &Event);
		const char *InputString() const;

		template<int N>
		void FormatDisplayValue(const char *pValue, char (&aOut)[N]);

		 
		std::shared_ptr<IMapSetting> m_pCurrentSetting;  
		std::vector<SCurrentSettingArg> m_vCurrentArgs;  
		int m_CursorArgIndex;  
		std::vector<SPossibleValueMatch> m_vPossibleMatches;  
		size_t m_LastCursorOffset;  
		CLineInput *m_pLineInput;
		char m_aCommand[128];  
		SCommandParseError m_Error;  
		int m_CommentOffset;  

		CMapSettingsBackend *m_pBackend;
		std::string m_CompositionStringBuffer;
		float m_FontSize;
	};

	CContext NewContext(CLineInput *pLineInput)
	{
		return CContext(this, pLineInput);
	}

private:
	 
	void LoadAllMapSettings();
	void LoadCommand(const char *pName, const char *pArgs, const char *pHelp);
	void LoadSettingInt(const std::shared_ptr<SMapSettingInt> &pSetting);
	void LoadSettingCommand(const std::shared_ptr<SMapSettingCommand> &pSetting);
	void InitValueLoaders();
	void LoadPossibleValues(const CSettingValuesBuilder &Builder, const std::shared_ptr<IMapSetting> &pSetting);
	void RegisterLoader(const char *pSettingName, const FLoaderFunction &pfnLoader);
	void LoadConstraints();

	static void PossibleConfigVariableCallback(const struct SConfigVariable *pVariable, void *pUserData);

	 
	using TArgumentConstraints = std::map<int, EArgConstraint>;  
	using TCommandArgumentConstraints = std::map<std::string, TArgumentConstraints>;  

	 
	 
	 
	 
	 
	 
	 
	 
	 
	 

	class CCommandArgumentConstraintBuilder;

	class CArgumentConstraintsBuilder
	{
		friend class CCommandArgumentConstraintBuilder;

	private:
		CArgumentConstraintsBuilder(TArgumentConstraints *pContainer) :
			m_pContainer(pContainer) {}

		TArgumentConstraints *m_pContainer;

	public:
		CArgumentConstraintsBuilder &Multiple(int Arg)
		{
			 
			(*m_pContainer)[Arg] = EArgConstraint::MULTIPLE;
			return *this;
		}

		CArgumentConstraintsBuilder &Unique(int Arg)
		{
			 
			(*m_pContainer)[Arg] = EArgConstraint::UNIQUE;
			return *this;
		}
	};

	class CCommandArgumentConstraintBuilder
	{
	public:
		CCommandArgumentConstraintBuilder(TCommandArgumentConstraints *pContainer) :
			m_pContainer(pContainer) {}

		CArgumentConstraintsBuilder operator()(const char *pSettingName, int ArgCount)
		{
			for(int i = 0; i < ArgCount; i++)
				(*m_pContainer)[pSettingName][i] = EArgConstraint::DEFAULT;
			return CArgumentConstraintsBuilder(&(*m_pContainer)[pSettingName]);
		}

	private:
		TCommandArgumentConstraints *m_pContainer;
	};

	TCommandArgumentConstraints m_ArgConstraintsPerCommand;

	 
	std::vector<std::shared_ptr<IMapSetting>> m_vpMapSettings;
	std::map<std::shared_ptr<IMapSetting>, std::vector<SParsedMapSettingArg>> m_ParsedCommandArgs;  
	TSettingsArgumentValues m_PossibleValuesPerCommand;
	std::map<std::string, FLoaderFunction> m_LoaderFunctions;

	friend class CEditor;

	 
	struct SLoadedMapSettings
	{
		std::vector<SInvalidSetting> m_vSettingsInvalid;
		std::vector<CEditorMapSetting> m_vSettingsValid;
		std::map<int, std::vector<int>> m_SettingsDuplicate;

		SLoadedMapSettings() = default;

		void Reset()
		{
			m_vSettingsInvalid.clear();
			m_vSettingsValid.clear();
			m_SettingsDuplicate.clear();
		}

	} m_LoadedMapSettings;
};

#endif
