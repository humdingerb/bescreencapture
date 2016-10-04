#include "BSCApp.h"
#include "BSCWindow.h"
#include "Controller.h"
#include "DeskbarControlView.h"
#include "Settings.h"

#include <private/interface/AboutWindow.h>
#include <Deskbar.h>
#include <StringList.h>

#include <stdio.h>


const char kChangeLog[] = {
#include "Changelog.h"
};

const char* kAuthors[] = {
	"Stefano Ceccherini"
};

int
main()
{
	BSCApp app;
	app.Run();
	
	return 0;
}


BSCApp::BSCApp()
	:
	BApplication(kAppSignature),
	fWindow(NULL)
{
	Settings::Load();

	fShouldStartRecording = false;
	gControllerLooper = new Controller();
}


BSCApp::~BSCApp()
{
	BDeskbar().RemoveItem("BSC Control");
	Settings::Save();
	
	gControllerLooper->Lock();
	gControllerLooper->Quit();
}


void
BSCApp::ReadyToRun()
{
	try {
		fWindow = new BSCWindow();
	} catch (...) {
		PostMessage(B_QUIT_REQUESTED);
		return;	
	}
	
	if (fShouldStartRecording) {
		fWindow->Run();
		BMessenger(fWindow).SendMessage(kCmdToggleRecording);
	} else
		fWindow->Show();

	
	BDeskbar deskbar;
	if (deskbar.IsRunning()) { 
		while (deskbar.HasItem("BSC Control"))
			deskbar.RemoveItem("BSC Control");
		if (!Settings().HideDeskbarIcon()) {
			deskbar.AddItem(new DeskbarControlView(BRect(0, 0, 15, 15),
				"BSC Control"));
		}
	}
}


bool
BSCApp::QuitRequested()
{
	return BApplication::QuitRequested();
}


void
BSCApp::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case kCmdToggleRecording:
			if (fWindow != NULL)
				BMessenger(fWindow).SendMessage(message);
			else {
				// Start recording as soon as a window is created
				fShouldStartRecording = true;
			}
			break;
		default:
			BApplication::MessageReceived(message);
			break;
	}
}


BStringList
SplitChangeLog(const char* changeLog)
{
	BStringList list;
	char* stringStart = (char*)changeLog;
	int i = 0;
	char c;
	while ((c = stringStart[i]) != '\0') {
		if (c == '-'  && i > 2 && stringStart[i - 1] == '-' && stringStart[i - 2] == '-') {
			BString string;
			string.Append(stringStart, i - 2);
			string.RemoveAll("\t");
			string.ReplaceAll("- ", "\n- ");			
			list.Add(string);
			stringStart = stringStart + i + 1;
			i = 0;
		} else
			i++;
	}
	return list;		
}


void
BSCApp::AboutRequested()
{
	BAboutWindow* aboutWindow = new BAboutWindow("BeScreenCapture", kAppSignature);
	aboutWindow->AddDescription("BeScreenCapture lets you record what happens on your screen and save it to a clip in any media format supported by Haiku.");
	aboutWindow->AddAuthors(kAuthors);
	
	BStringList list = SplitChangeLog(kChangeLog);
	int32 stringCount = list.CountStrings();
	char** charArray = new char* [stringCount + 1];
	for (int32 i = 0; i < stringCount; i++) {
		charArray[i] = (char*)list.StringAt(i).String();
	}
	charArray[stringCount] = NULL;
	
	aboutWindow->AddVersionHistory((const char**)charArray);
	delete[] charArray;
	
	aboutWindow->Show();
}

