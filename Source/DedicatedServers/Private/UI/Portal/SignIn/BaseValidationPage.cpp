#include "UI/Portal/SignIn/BaseValidationPage.h"

bool UBaseValidationPage::IsStrongPassword(const FString& Password, FString& StatusMessage)
{
	const FRegexPattern NumberPattern(TEXT(R"(\d)")); // contains at least one number
	const FRegexPattern SpecialCharPattern(TEXT(R"([^\w\s])")); // contains at least one special character
	const FRegexPattern UpperCasePattern(TEXT(R"([A-Z])")); // contains at least one uppercase letter
	const FRegexPattern LowerCasePattern(TEXT(R"([a-z])")); // contains at least one lowercase letter

	FRegexMatcher NumberMatcher(NumberPattern, Password);
	FRegexMatcher SpecialCharMatcher(SpecialCharPattern, Password);
	FRegexMatcher UpperCaseMatcher(UpperCasePattern, Password);
	FRegexMatcher LowerCaseMatcher(LowerCasePattern, Password);

	if (!NumberMatcher.FindNext())
	{
		StatusMessage = TEXT("Password must contain at least 1 number");
		return false;
	}
	if (!SpecialCharMatcher.FindNext())
	{
		StatusMessage = TEXT("Password must contain at least 1 special character");
		return false;
	}
	if (!UpperCaseMatcher.FindNext())
	{
		StatusMessage = TEXT("Password must contain at least 1 uppercase letter");
		return false;
	}
	if (!LowerCaseMatcher.FindNext())
	{
		StatusMessage = TEXT("Password must contain at least 1 lowercase letter");
		return false;
	}
	return true;
}

bool UBaseValidationPage::IsValidEmail(const FString& Email)
{
	//const FRegexPattern EmailPattern(TEXT("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,64}$"));
	const FRegexPattern EmailPattern(TEXT(R"((^[^\s@]+@[^\s@]+\.[^\s@]{2,}$))"));
	FRegexMatcher Matcher(EmailPattern, Email);

	return Matcher.FindNext();
}

bool UBaseValidationPage::IsValidConfirmationCode(const FText& ConfirmationCode)
{
	const FRegexPattern SixDigitsPattern(TEXT(R"(^\d{6}$)"));
	FRegexMatcher Matcher(SixDigitsPattern, ConfirmationCode.ToString());

	return Matcher.FindNext();
}
