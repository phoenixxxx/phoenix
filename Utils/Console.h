#pragma once
#include <map>
#include <Utils/Types.h>
#include <Utils/Event.h>
#include <ThirdParty/imgui/imgui.h>
// System includes
#include <ctype.h>          // toupper
#include <stdio.h>          // vsnprintf, sscanf, printf
#include <stdlib.h>         // NULL, malloc, free, atoi
#if defined(_MSC_VER) && _MSC_VER <= 1500 // MSVC 2008 or earlier
#include <stddef.h>         // intptr_t
#else
#endif
// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (disable: 4996)     // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#pragma warning (disable: 26451)    // [Static Analyzer] Arithmetic overflow : Using operator 'xxx' on a 4 byte value and then casting the result to a 8 byte value. Cast the value to the wider type before calling operator 'xxx' to avoid overflow(io.2).
#endif

//-----------------------------------------------------------------------------
// [SECTION] Example App: Debug Console / ShowExampleAppConsole()
//-----------------------------------------------------------------------------

// Demonstrate creating a simple console window, with scrolling, filtering, completion and history.
// For the console example, we are using a more C++ like approach of declaring a class to hold both data and functions.
namespace Phoenix
{
	struct Console
	{
	public:
		typedef Event<const std::vector<std::string>&> CommandEvent_t;

		char                  InputBuf[256];
		ImVector<char*>       Items;
		std::map<const char*, CommandEvent_t> Commands;
		ImVector<char*>       History;
		int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
		ImGuiTextFilter       Filter;
		bool                  AutoScroll;
		bool                  ScrollToBottom;
		Size                  mParentWindowSize;
		enum class LogType { eInfo, eWarning, eError };

		static Console* Instance()
		{
			static Console instance;
			return &instance;
		}
		bool Resize(const Size& newSize)
		{
			mParentWindowSize = newSize;
			return true;
		}
		void RegisterCommand(const char* name, const CommandEvent_t::Callback& handler)
		{
			Commands[name] += handler;
		}
		bool Initialize()
		{
			ClearLog();
			memset(InputBuf, 0, sizeof(InputBuf));
			HistoryPos = -1;

			// "CLASSIFY" is here to provide the test case where "C"+[tab] completes to "CL" and display multiple matches.
			//Commands.push_back("HELP");
			//Commands.push_back("HISTORY");
			//Commands.push_back("CLEAR");
			//Commands.push_back("CLASSIFY");
			AutoScroll = true;
			ScrollToBottom = false;
			//Log(LogType::eInfo, "Welcome to Dear ImGui!");
			return true;
		}
		~Console()
		{
			ClearLog();
			for (int i = 0; i < History.Size; i++)
				free(History[i]);
		}

		// Portable helpers
		static int   Stricmp(const char* s1, const char* s2) { int d; while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; } return d; }
		static int   Strnicmp(const char* s1, const char* s2, int n) { int d = 0; while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; n--; } return d; }
		static char* Strdup(const char* s) { IM_ASSERT(s); size_t len = strlen(s) + 1; void* buf = malloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)s, len); }
		static void  Strtrim(char* s) { char* str_end = s + strlen(s); while (str_end > s && str_end[-1] == ' ') str_end--; *str_end = 0; }

		void    ClearLog()
		{
			for (int i = 0; i < Items.Size; i++)
				free(Items[i]);
			Items.clear();
		}	

		void    Log(LogType type, const char* fmt, ...) IM_FMTARGS(2)
		{
			const char* header="";
			size_t headerLen;
			switch (type)
			{
			case LogType::eWarning:
				header = "[warn] ";
				break;
			case LogType::eError:
				header = "[error] ";
				break;
			}
			headerLen = std::strlen(header);

			char buf[2048];
			std::sprintf(buf, header);
			va_list args;
			va_start(args, fmt);
			vsnprintf(&buf[headerLen], IM_ARRAYSIZE(buf), fmt, args);
			buf[IM_ARRAYSIZE(buf) - 1] = 0;
			va_end(args);
			Items.push_back(Strdup(buf));
		}
		void    Draw(const char* title = "Console")
		{
			static bool closed = false;

			float height = mParentWindowSize.mHeight * .32f;
			float y = mParentWindowSize.mHeight - height;

			ImGui::SetNextWindowSize(ImVec2(mParentWindowSize.mWidth, height));
			if (!closed)
				ImGui::SetNextWindowPos(ImVec2(0, y));	
			else
				ImGui::SetNextWindowPos(ImVec2(0, mParentWindowSize.mHeight - 19));

			if (!ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoResize))
			{
				closed = true;
				ImGui::End();
				return;
			}
			else
				closed = false;

			//ImGui::TextWrapped(
			//	"This example implements a console with basic coloring, completion (TAB key) and history (Up/Down keys). A more elaborate "
			//	"implementation may want to store entries along with extra data such as timestamp, emitter, etc.");
			//ImGui::TextWrapped("Enter 'HELP' for help.");

			//// TODO: display items starting from the bottom

			//if (ImGui::SmallButton("Add Debug Text")) { AddLog("%d some text", Items.Size); AddLog("some more text"); AddLog("display very important message here!"); }
			//ImGui::SameLine();
			if (ImGui::SmallButton("Clear")) { ClearLog(); }
			ImGui::SameLine();
			if (ImGui::SmallButton("History")) 
			{
				int first = History.Size - 10;
				for (int i = first > 0 ? first : 0; i < History.Size; i++)
					Log(LogType::eInfo, "%3d: %s\n", i, History[i]);
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("Help")) 
			{
				Log(LogType::eInfo, "Commands:");
				for (auto& cmd : Commands)//int i = 0; i < Commands.Size; i++)
					Log(LogType::eInfo, "- %s", cmd.first);// Commands[i]);
			}
			ImGui::SameLine();
			bool copy_to_clipboard = ImGui::SmallButton("Copy");
			//static float t = 0.0f; if (ImGui::GetTime() - t > 0.02f) { t = ImGui::GetTime(); AddLog("Spam %f", t); }

			ImGui::Separator();

#if 0
			// Options menu
			if (ImGui::BeginPopup("Options"))
			{
				ImGui::Checkbox("Auto-scroll", &AutoScroll);
				ImGui::EndPopup();
			}

			// Options, Filter
			if (ImGui::Button("Options"))
				ImGui::OpenPopup("Options");

			ImGui::SameLine();
#endif

			Filter.Draw("Filter (\"incl,-excl\")", 180);
			ImGui::Separator();


			// Reserve enough left-over height for 1 separator + 1 input text
			const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
			ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
			if (ImGui::BeginPopupContextWindow())
			{
				if (ImGui::Selectable("Clear")) ClearLog();
				ImGui::EndPopup();
			}

			// Display every line as a separate entry so we can change their color or add custom widgets.
			// If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
			// NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping
			// to only process visible items. The clipper will automatically measure the height of your first item and then
			// "seek" to display only items in the visible area.
			// To use the clipper we can replace your standard loop:
			//      for (int i = 0; i < Items.Size; i++)
			//   With:
			//      ImGuiListClipper clipper;
			//      clipper.Begin(Items.Size);
			//      while (clipper.Step())
			//         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
			// - That your items are evenly spaced (same height)
			// - That you have cheap random access to your elements (you can access them given their index,
			//   without processing all the ones before)
			// You cannot this code as-is if a filter is active because it breaks the 'cheap random-access' property.
			// We would need random-access on the post-filtered list.
			// A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices
			// or offsets of items that passed the filtering test, recomputing this array when user changes the filter,
			// and appending newly elements as they are inserted. This is left as a task to the user until we can manage
			// to improve this example code!
			// If your items are of variable height:
			// - Split them into same height items would be simpler and facilitate random-seeking into your list.
			// - Consider using manual call to IsRectVisible() and skipping extraneous decoration from your items.
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
			if (copy_to_clipboard)
				ImGui::LogToClipboard();
			for (int i = 0; i < Items.Size; i++)
			{
				const char* item = Items[i];
				if (!Filter.PassFilter(item))
					continue;

				// Normally you would store more information in your item than just a string.
				// (e.g. make Items[] an array of structure, store color/type etc.)
				ImVec4 color;
				bool has_color = false;
				if (strstr(item, "[error]")) { color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); has_color = true; }
				else if (strstr(item, "[warn]")) { color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f); has_color = true; }
				else if (strncmp(item, "# ", 2) == 0) { color = ImVec4(0.0f, 0.8f, 0.0f, 1.0f); has_color = true; }

				if (has_color)
					ImGui::PushStyleColor(ImGuiCol_Text, color);
				ImGui::TextUnformatted(item);
				if (has_color)
					ImGui::PopStyleColor();
			}
			if (copy_to_clipboard)
				ImGui::LogFinish();

			if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
				ImGui::SetScrollHereY(1.0f);
			ScrollToBottom = false;

			ImGui::PopStyleVar();
			ImGui::EndChild();
			ImGui::Separator();

			// Command-line
			bool reclaim_focus = false;
			ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
			if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), input_text_flags, &TextEditCallbackStub, (void*)this))
			{
				char* s = InputBuf;
				Strtrim(s);
				if (s[0])
					ExecCommand(s);
				strcpy(s, "");
				reclaim_focus = true;
			}

			// Auto-focus on window apparition
			ImGui::SetItemDefaultFocus();
			if (reclaim_focus)
				ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

			ImGui::End();
		}

		void    ExecCommand(const char* command_line)
		{
			// Insert into history. First find match and delete it so it can be pushed to the back.
			// This isn't trying to be smart or optimal.
			HistoryPos = -1;
			for (int i = History.Size - 1; i >= 0; i--)
				if (Stricmp(History[i], command_line) == 0)
				{
					free(History[i]);
					History.erase(History.begin() + i);
					break;
				}
			History.push_back(Strdup(command_line));

			// Process command
			// Locate beginning of current word
			std::vector<std::string> words;
			std::string word;
			while (*command_line)
			{
				const char c = *command_line;
				if (c == ' ' || c == '\t' || c == ',' || c == ';')
				{
					words.push_back(word);
					word.clear();
				}
				else
					word += c;
				command_line++;
			}

			bool found = false;
			if (word.size() != 0)
			{
				words.push_back(word);//get the last word

				for (const auto& cmd : Commands)
				{
					if (Stricmp(words[0].c_str(), cmd.first) == 0)
					{
						found = true;
						cmd.second(words);
						break;
					}
				}	
			}

			if (found)
				Log(LogType::eInfo, "# %s\n", History.back());
			else
				Log(LogType::eWarning, "Unknown command: '%s'\n", words[0].c_str());

			// On command input, we scroll to bottom even if AutoScroll==false
			ScrollToBottom = true;
		}

		// In C++11 you'd be better off using lambdas for this sort of forwarding callbacks
		static int TextEditCallbackStub(ImGuiInputTextCallbackData* data)
		{
			Console* console = (Console*)data->UserData;
			return console->TextEditCallback(data);
		}

		int     TextEditCallback(ImGuiInputTextCallbackData* data)
		{
			//AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
			switch (data->EventFlag)
			{
			case ImGuiInputTextFlags_CallbackCompletion:
			{
				// Example of TEXT COMPLETION

				// Locate beginning of current word
				const char* word_end = data->Buf + data->CursorPos;
				const char* word_start = word_end;
				while (word_start > data->Buf)
				{
					const char c = word_start[-1];
					if (c == ' ' || c == '\t' || c == ',' || c == ';')
						break;
					word_start--;
				}

				// Build a list of candidates
				ImVector<const char*> candidates;
				for (const auto& cmd : Commands)
					if (Strnicmp(cmd.first, word_start, (int)(word_end - word_start)) == 0)
						candidates.push_back(cmd.first);

				if (candidates.Size == 0)
				{
					// No match
					Log(LogType::eInfo, "No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
				}
				else if (candidates.Size == 1)
				{
					// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
					data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
					data->InsertChars(data->CursorPos, candidates[0]);
					data->InsertChars(data->CursorPos, " ");
				}
				else
				{
					// Multiple matches. Complete as much as we can..
					// So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
					int match_len = (int)(word_end - word_start);
					for (;;)
					{
						int c = 0;
						bool all_candidates_matches = true;
						for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
							if (i == 0)
								c = toupper(candidates[i][match_len]);
							else if (c == 0 || c != toupper(candidates[i][match_len]))
								all_candidates_matches = false;
						if (!all_candidates_matches)
							break;
						match_len++;
					}

					if (match_len > 0)
					{
						data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
						data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
					}

					// List matches
					Log(LogType::eInfo, "Possible matches:\n");
					for (int i = 0; i < candidates.Size; i++)
						Log(LogType::eInfo, "- %s\n", candidates[i]);
				}

				break;
			}
			case ImGuiInputTextFlags_CallbackHistory:
			{
				// Example of HISTORY
				const int prev_history_pos = HistoryPos;
				if (data->EventKey == ImGuiKey_UpArrow)
				{
					if (HistoryPos == -1)
						HistoryPos = History.Size - 1;
					else if (HistoryPos > 0)
						HistoryPos--;
				}
				else if (data->EventKey == ImGuiKey_DownArrow)
				{
					if (HistoryPos != -1)
						if (++HistoryPos >= History.Size)
							HistoryPos = -1;
				}

				// A better implementation would preserve the data on the current input line along with cursor position.
				if (prev_history_pos != HistoryPos)
				{
					const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
					data->DeleteChars(0, data->BufTextLen);
					data->InsertChars(0, history_str);
				}
			}
			}
			return 0;
		}
	};
}