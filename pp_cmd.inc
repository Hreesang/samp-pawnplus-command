// -
// Include Guards
// -

#if defined _INC_pp_cmd
	#undef _INC_pp_cmd
#endif

#if defined _pp_cmd_included
	#endinput
#endif
#define _pp_cmd_included


// -
// External Packages
// -

#tryinclude <PawnPlus>
#if !defined _PawnPlus_included
  #error You need PawnPlus to use this library (https://github.com/IS4Code/PawnPlus).
#endif


// -
// Pre-Hooks
// -

#if !defined CHAIN_ORDER
	#define CHAIN_ORDER() 0
#endif

#define CHAIN_HOOK(%0) forward @CO_%0();public @CO_%0(){return CHAIN_ORDER()+1;}
#define CHAIN_NEXT(%0) @CO_%0

#define CHAIN_FORWARD:%0_%2(%1)=%3; \
	forward %0_%2(%1); \
	public %0_%2(%1) <_ALS : _ALS_x0, _ALS : _ALS_x1> { return (%3); } \
	public %0_%2(%1) <> { return (%3); }

#define CHAIN_PUBLIC:%0(%1) %0(%1) <_ALS : _ALS_go>

CHAIN_HOOK(PPC)
#undef CHAIN_ORDER
#define CHAIN_ORDER CHAIN_NEXT(PPC)

static stock _PPCMD_IncludeStates() <_ALS : _ALS_x0, _ALS : _ALS_x1, _ALS : _ALS_x2, _ALS : _ALS_x3> {}
static stock _PPCMD_IncludeStates() <_ALS : _ALS_go> {}


// -
// Declarations
// -

#if !defined INVALID_COMMAND_ID
	#define INVALID_COMMAND_ID			-1
#endif

#if !defined MAX_COMMAND_FUNC_NAME
	#define MAX_COMMAND_FUNC_NAME 		32
#endif

#if !defined MAX_COMMAND_NAME
	#define MAX_COMMAND_NAME 			28
#endif

#if !defined CMD_FAILURE				
	#define CMD_FAILURE					0
#endif

#if !defined CMD_SUCCESS
	#define CMD_SUCCESS					1
#endif


// Macros
#define COMMAND:%1(%2) \
			forward cmd_%1(%2); \
			public cmd_%1(%2)
			
#define CMD:%1(%2) \
			COMMAND:%1(%2)

#define command(%1,%2,%3) \
			COMMAND:%1(%2, %3)

#define cmd(%1,%2,%3) \
			COMMAND:%1(%2, %3)

#define ALIAS:%0(%1); \
			forward altc_%0(); \
			public altc_%0() { return AddCommandAlias(GetCommandID(#%0), %1), 1; }

#define INIT:%0() \
			forward intc_%0(); \
			public intc_%0()

#if !defined isnull
	#define isnull(%1) \
				((!(%1[0])) || (((%1[0]) == '\1') && (!(%1[1]))))
#endif


static enum _:E_CMD_INFO {
	E_CMD_NAME[MAX_COMMAND_NAME],
	E_CMD_FUNC_NAME[MAX_COMMAND_FUNC_NAME],
	E_CMD_POINTING_ID,
}

static stock List:gs_commandsList = INVALID_LIST;
static stock gs_runningCommandID = INVALID_COMMAND_ID;


// -
// Static APIs
// -

static void:PPC_InitializeCommands()
{
	new totalPublics = amx_num_publics();

	new const cmdPrefix[] = "cmd_";
	new const cmdPrefixLen = sizeof cmdPrefix;

	// Initialize commands...
	for (new i = 0; i < totalPublics; i++) {
		new publicName[MAX_COMMAND_FUNC_NAME];
		amx_public_name(i, publicName);

		if (strfind(publicName, cmdPrefix) != 0) {
			continue;
		}

		new cmdName[MAX_COMMAND_NAME];
		strmid(cmdName, publicName, cmdPrefixLen - 1, strlen(publicName));

		new cmdInfo[E_CMD_INFO];
		strcat(cmdInfo[E_CMD_NAME], cmdName, MAX_COMMAND_NAME);
		strcat(cmdInfo[E_CMD_FUNC_NAME], publicName, MAX_COMMAND_FUNC_NAME);
		cmdInfo[E_CMD_POINTING_ID] = INVALID_COMMAND_ID;
		
		if (gs_commandsList == INVALID_LIST) {
			gs_commandsList = list_new();
		}

		list_add_arr(gs_commandsList, cmdInfo);
	}
}

static void:PPC_InitializeAliasCommands()
{
	new const aliasFuncPrefix[] = "altc_";
	for (new i = 0, j = GetTotalCommands(); i < j; i++) {
		new cmdInfo[E_CMD_INFO];
		list_get_arr(gs_commandsList, i, cmdInfo);

		if (cmdInfo[E_CMD_POINTING_ID] != INVALID_COMMAND_ID) {
			continue;
		}

		new publicName[MAX_COMMAND_FUNC_NAME];
		strcat(publicName, aliasFuncPrefix, MAX_COMMAND_FUNC_NAME);
		strcat(publicName, cmdInfo[E_CMD_NAME]);

		new result;
		pawn_try_call_public(publicName, result, "");
		#pragma unused result
	}
}

static void:PPC_CallCommandsInitializer()
{
	new const initFuncPrefix[] = "intc_";
	for (new i = 0, j = GetTotalCommands(); i < j; i++) {
		new cmdInfo[E_CMD_INFO];
		list_get_arr(gs_commandsList, i, cmdInfo);

		if (cmdInfo[E_CMD_POINTING_ID] != INVALID_COMMAND_ID) {
			continue;
		}

		new publicName[MAX_COMMAND_FUNC_NAME];
		strcat(publicName, initFuncPrefix, MAX_COMMAND_FUNC_NAME);
		strcat(publicName, cmdInfo[E_CMD_NAME]);

		new result;
		gs_runningCommandID = i;
	
		pawn_try_call_public(publicName, result, "");
		#pragma unused result

		gs_runningCommandID = INVALID_COMMAND_ID;
	}
}

static PPC_ParseCommandText(const cmdtext[], cmd[], params[], cmdLen = sizeof cmd, paramsLen = sizeof params)
{
	cmd[0] = EOS;
	params[0] = EOS;

	#if defined _INC_SSCANF
		sscanf(cmdtext[1], "s[*]S()[*]", cmdLen, cmd, paramsLen, params);
	#else
		new pos;
		if((pos = strfind(cmdtext, " ", true, 1)) == -1) {
			strcat(cmd, cmdtext[1], cmdLen);
		} else {
			strmid(cmd, cmdtext, 1, pos, cmdLen);
			while (cmdtext[pos] == ' ') {
				pos++;
			}

			if (pos < strlen(cmdtext)) {
				strcat(params, cmdtext[pos], paramsLen);
			}
		}
	#endif
}


// -
// APIs
// -

forward OnPlayerCommandReceived(playerid, cmdtext[]); 
forward OnPlayerCommandPerformed(playerid, cmdtext[], success); 

stock GetTotalCommands()
{
	if (gs_commandsList == INVALID_LIST) {
		return 0;
	}

	return list_size(gs_commandsList);
}

stock GetCommandID(const cmd[])
{
	new cmdid = INVALID_COMMAND_ID;
	new totalCommands = GetTotalCommands();

	for (new i = 0; i < totalCommands; i++) {
		new cmdInfo[E_CMD_INFO];
		list_get_arr(gs_commandsList, i, cmdInfo);

		if (strcmp(cmdInfo[E_CMD_NAME], cmd) == 0) {
			cmdid = i;
			break;
		}
	}
	return cmdid;
}

stock GetRunningCommandID()
{
	return gs_runningCommandID;
}

stock bool:IsValidCommand(cmdid)
{
	return (0 <= cmdid < GetTotalCommands());
}

stock bool:GetCommandName(cmdid, output[], len = sizeof output)
{
	if (!IsValidCommand(cmdid)) {
		return false;
	}

	new cmdInfo[E_CMD_INFO];
	list_get_arr(gs_commandsList, cmdid, cmdInfo);

	output[0] = EOS;
	strcat(output, cmdInfo[E_CMD_NAME], len);

	return true;
}

stock bool:GetCommandFunctionName(cmdid, output[], len = sizeof output)
{
	if (!IsValidCommand(cmdid)) {
		return false;
	}

	new cmdInfo[E_CMD_INFO];
	list_get_arr(gs_commandsList, cmdid, cmdInfo);

	output[0] = EOS;
	strcat(output, cmdInfo[E_CMD_FUNC_NAME], len);
	
	return true;
}

stock bool:SetCommandFunctionName(cmdid, const funcname[])
{
	if (!IsValidCommand(cmdid)) {
		return false;
	}

	new cmdInfo[E_CMD_INFO];
	list_get_arr(gs_commandsList, cmdid, cmdInfo);

	cmdInfo[E_CMD_FUNC_NAME][0] = EOS;
	strcat(cmdInfo[E_CMD_FUNC_NAME], funcname, MAX_COMMAND_FUNC_NAME);

	list_set_arr(gs_commandsList, cmdid, cmdInfo);
	return true;
}

stock bool:AddCommandAlias(cmdid, ...)
{
	if (!IsValidCommand(cmdid)) {
		return false;
	}

	new originalCmdInfo[E_CMD_INFO];
	list_get_arr(gs_commandsList, cmdid, originalCmdInfo);

	new args = numargs();
	for (new arg = 1; arg < args; arg++) {
		new aliasName[MAX_COMMAND_NAME];
		new pos;
		new c;

		while ((c = getarg(arg, pos)) != '\0' && pos < sizeof aliasName) {
			aliasName[pos] = c;
			pos++;
		}

		new cmdInfo[E_CMD_INFO];
		strcat(cmdInfo[E_CMD_NAME], aliasName, MAX_COMMAND_NAME);
		strcat(cmdInfo[E_CMD_FUNC_NAME], originalCmdInfo[E_CMD_FUNC_NAME], MAX_COMMAND_FUNC_NAME);
		list_add_arr(gs_commandsList, cmdInfo);
	}

	return true;
}

stock bool:IsCommandAlias(cmdid)
{
	if (!IsValidCommand(cmdid)) {
		return false;
	}

	new cmdInfo[E_CMD_INFO];
	list_get_arr(gs_commandsList, cmdid, cmdInfo);

	return cmdInfo[E_CMD_POINTING_ID] != INVALID_COMMAND_ID;
}

stock GetCommandPointingID(cmdid)
{
	if (!IsValidCommand(cmdid)) {
		return INVALID_COMMAND_ID;
	}

	new cmdInfo[E_CMD_INFO];
	list_get_arr(gs_commandsList, cmdid, cmdInfo);

	return cmdInfo[E_CMD_POINTING_ID];
}

stock List:GetCommandAlias(cmdid)
{
	if (!IsValidCommand(cmdid)) {
		return INVALID_LIST;
	}

	new totalCommands = GetTotalCommands();
	new List:result = list_new();

	for (new i = 0; i < totalCommands; i++) {
		new cmdInfo[E_CMD_INFO];
		list_get_arr(gs_commandsList, cmdid, cmdInfo);

		if (cmdInfo[E_CMD_POINTING_ID] != cmdid) {
			continue;
		}
		
		list_add(result, i);
	}

	if (!list_size(result)) {
		list_delete(result);
		return INVALID_LIST;
	}

	return result;
}

stock EmulateCommand(playerid, const cmd[], const params[] = "", cmdid = INVALID_COMMAND_ID)
{
	if (cmdid == INVALID_COMMAND_ID) {
		cmdid = GetCommandID(cmd);
	}

	new cmdtext[MAX_COMMAND_NAME + (128-MAX_COMMAND_NAME)] = "/";
	strcat(cmdtext, cmd);
	if (!isnull(params)) {
		strcat(cmdtext, " ");
		strcat(cmdtext, params);
	}

	if (!IsValidCommand(cmdid)) {
		return OnPlayerCommandPerformed(playerid, cmdtext, CMD_FAILURE);
	}

	gs_runningCommandID = cmdid;

	new cmdInfo[E_CMD_INFO];
	list_get_arr(gs_commandsList, cmdid, cmdInfo);

	new cmdret = pawn_call_public(cmdInfo[E_CMD_FUNC_NAME], "is", playerid, isnull(params) ? "\1" : params);
	new ret = OnPlayerCommandPerformed(playerid, cmdtext, cmdret);

	gs_runningCommandID = INVALID_COMMAND_ID;
	return ret;
}


// -
// Internals
// -

#if defined FILTERSCRIPT
	public OnFilterScriptInit()
	{
		state _ALS : _ALS_go;
		PPC_InitializeCommands();
		PPC_InitializeAliasCommands();
		PPC_CallCommandsInitializer();

		return PPC_OnFilterScriptInit();
	}
#else
	public OnGameModeInit()
	{
		state _ALS : _ALS_go;
		PPC_InitializeCommands();
		PPC_InitializeAliasCommands();
		PPC_CallCommandsInitializer();

		return PPC_OnGameModeInit();
	}
#endif

public OnPlayerCommandText(playerid, cmdtext[])
{
	if (!PPC_OnPlayerCommandText(playerid, cmdtext)) {
		return 0;
	}

	if (!OnPlayerCommandReceived(playerid, cmdtext)) {
		return 1;
	}

	new cmd[MAX_COMMAND_NAME];
	new params[128];

	PPC_ParseCommandText(cmdtext, cmd, params);
	return EmulateCommand(playerid, cmd, params);
}

public OnPlayerCommandReceived(playerid, cmdtext[])
{
	return PPC_OnPlayerCommandReceived(playerid, cmdtext);
}

public OnPlayerCommandPerformed(playerid, cmdtext[], success)
{
	return PPC_OnPlayerCommandPerformed(playerid, cmdtext, success);
}


// -
// Hooks
// -

#if defined FILTERSCRIPT
	#if defined _ALS_OnFilterScriptInit
		#undef OnFilterScriptInit
	#else
		#define _ALS_OnFilterScriptInit
	#endif
	#define OnFilterScriptInit(%0) CHAIN_PUBLIC:PPC_OnFilterScriptInit(%0)
	CHAIN_FORWARD:PPC_OnFilterScriptInit() = 1;
#else
	#if defined _ALS_OnGameModeInit
		#undef OnGameModeInit
	#else
		#define _ALS_OnGameModeInit
	#endif
	#define OnGameModeInit(%0) CHAIN_PUBLIC:PPC_OnGameModeInit(%0)
	CHAIN_FORWARD:PPC_OnGameModeInit() = 1;
#endif

#if defined _ALS_OnPlayerCommandText
	#undef OnPlayerCommandText
#else
	#define _ALS_OnPlayerCommandText
#endif
#define OnPlayerCommandText(%0) CHAIN_PUBLIC:PPC_OnPlayerCommandText(%0)
CHAIN_FORWARD:PPC_OnPlayerCommandText(playerid, cmdtext[]) = 1;

#if defined _ALS_OnPlayerCommandReceived
	#undef OnPlayerCommandReceived
#else
	#define _ALS_OnPlayerCommandReceived
#endif
#define OnPlayerCommandReceived(%0) CHAIN_PUBLIC:PPC_OnPlayerCommandReceived(%0)
CHAIN_FORWARD:PPC_OnPlayerCommandReceived(playerid, cmdtext[]) = 1;

#if defined _ALS_OnPlayerCommandPerformed
	#undef OnPlayerCommandPerformed
#else
	#define _ALS_OnPlayerCommandPerformed
#endif
#define OnPlayerCommandPerformed(%0) CHAIN_PUBLIC:PPC_OnPlayerCommandPerformed(%0)
CHAIN_FORWARD:PPC_OnPlayerCommandPerformed(playerid, cmdtext[], success) = 1;