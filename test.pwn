// -
// External Packages
// -

#include <crashdetect>
#include <test-boilerplate>
#include <sscanf2>


// -
// Internal Packages
// -

#include "pp_cmd"

#include <YSI_Coding\y_va>
#include <YSI_Coding\y_hooks>
#include <YSI_Server\y_colors>


// -
// Internals
// -

main() {}

static gs_processTick;

hook OnPlayerCommandText(playerid, cmdtext[])
{
	gs_processTick = GetTickCount();
	return 1;
}

hook OnPlayerCommandPerformed(playerid, cmdtext[], success)
{
	new string[144];
	format(
		string, 
		sizeof string, 
		"[Debug] "#LIGHTGREY"Command \"%s\" is executed, success: %s (time taken: %ims).", 
		cmdtext, 
		success ? "true" : "false",
		GetTickCount()-gs_processTick
	);
	SendClientMessage(playerid, X11_GOLD, string);

	if (success == CMD_FAILURE) {
		SendClientMessage(playerid, X11_TOMATO, "[Error] "#GREY"That command doesn't exist!");
	}

	return 1;
}

INIT:wow()
{
	printf("Command \"/wow\" with ID %i is initialized.", GetRunningCommandID());
	return 1;
}

ALIAS:wow("w", "wo");
CMD:wow(playerid, const params[])
{
	SendClientMessage(playerid, X11_GOLD, "Hello, my brother. You're wow-ing, aren't you?");
	return 1;
}

INIT:commands()
{
	printf("Command \"/commands\" with ID %i is initialized.", GetRunningCommandID());
	return 1;
}

ALIAS:commands("help", "cmds", "cmd");
CMD:commands(playerid, const params[])
{
	new totalCommands = GetTotalCommands();
	va_SendClientMessage(playerid, X11_LIMEGREEN, "This server has %i commands:", totalCommands);

	for (new i = 0; i < totalCommands; i++) {
		new List:alias = GetCommandAlias(i);

		new cmdName[MAX_COMMAND_NAME];
		GetCommandName(i, cmdName);

		if (alias != INVALID_LIST) {
			new string[128];
			for (new d = 0, j = list_size(alias); d < j; d++) {
				new aliasCmdid = list_get(alias, d);
				new aliasName[MAX_COMMAND_NAME];
				
				GetCommandName(aliasCmdid, aliasName);

				if (d == 0) {
					format(string, sizeof string, "(/%s", aliasName);
				} else {
					format(string, sizeof string, "%s, /%s", string, aliasName);
				}
			}
			strcat(string, ")");

			va_SendClientMessage(playerid, X11_LIMEGREEN, "Command ID %i - /%s %s.", i, cmdName, string);
			list_delete(alias);
		} else {
			va_SendClientMessage(playerid, X11_LIMEGREEN, "Command ID %i - /%s.", i, cmdName);
		}
	}

	return 1;
}


INIT:iam()
{
	printf("Command \"/iam\" with ID %i is initialized.", GetRunningCommandID());
	return 1;
}

CMD:iam(playerid, const params[])
{
	if (isnull(params)) {
		return SendClientMessage(playerid, X11_TOMATO, "USAGE: "#WHITE"/iam [name]");
	}

	va_SendClientMessage(playerid, X11_GOLD, "Oh, hi %s.", params);
	va_SendClientMessage(playerid, X11_GOLD, "Anyway, this command's ID is %i.", GetRunningCommandID());
	return 1;
}