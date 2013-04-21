#if defined _messagebot_included
	#endinput
#endif

#define __messagebot_included


// Do not touch, bad things will happen!
// Maximum Recipients
#define MAX_RECIPIENTS 300




enum MessageBotResult
{
	RESULT_NO_ERROR = 0, 	  // Everything worked fine
	RESULT_LOGIN_ERROR,   	  // Error while trying to login
	RESULT_TIMEOUT_ERROR,     // Operation timed out
	RESULT_RECIPIENTS_EMPTY,  // No recipients were setup prior to sending a message
}




/**
 * Called when the message was queued and the result is available.
 *
 * @param result    Result code.
 * @param error     Error code.
 * @noreturn
 */
funcenum MessageBotReceiveCB
{
    public(MessageBotResult:result, error)
};





/**
 * Sets the login data to connect to the account.
 *
 * @param steamUsername       Steam username.
 * @param steamPassword       Steam password.
 * @noreturn
 */
native MessageBot_SetLoginData(const String:steamUsername[], const String:steamPassword[]);





/**
 * Sends a message to all recipients.
 *
 * @param callback            Callback to be called when result is available.
 * @param message             Message to be sent.
 * @param showOnline 		  Show that bot is online?
 * @noreturn
 */
native MessageBot_SendMessage(MessageBotReceiveCB:callback, const String:message[], bool:showOnlien=true);




/**
 * Add an auth to the list of recipients.
 *
 * @param auth               Auth/CommunityID of the recipient.
 * @return                   True if auth didn't exist and was added to the list, false otherwise.
 */
native bool:MessageBot_AddRecipient(const String:auth[]);




/**
 * Remove an auth from the list of recipients.
 *
 * @param auth               Auth/CommunityID of the recipient.
 * @return                   True if auth existed and was removed from the list, false otherwise.
 */
native bool:MessageBot_RemoveRecipient(const String:auth[]);





/**
 * Returns whether the auth is on the recipients list.
 *
 * @param auth               Auth/CommunityID of the recipient.
 * @return                   True if auth is a recipient, false otherwise.
 */
native bool:MessageBot_IsRecipient(const String:auth[]);





/**
 * Clear the list of recipients.
 *
 * @noreturn
 */
native MessageBot_ClearRecipients();






/** Do Not Edit Below This Line **/
public Extension:__ext_messagebot =
{
	name = "messagebot",
	file = "messagebot.ext",

	#if defined AUTOLOAD_EXTENSIONS
		autoload = 1,
	#else
		autoload = 0,
	#endif

	#if defined REQUIRE_EXTENSIONS
		required = 1,
	#else
		required = 0,
	#endif
};

#if !defined REQUIRE_EXTENSIONS
	public __ext_messagebot_SetNTVOptional()
	{
		MarkNativeAsOptional("MessageBot_SetLoginData");
		MarkNativeAsOptional("MessageBot_SendMessage");
		MarkNativeAsOptional("MessageBot_AddRecipient");
		MarkNativeAsOptional("MessageBot_RemoveRecipient");
		MarkNativeAsOptional("MessageBot_ClearRecipients");
	}
#endif
