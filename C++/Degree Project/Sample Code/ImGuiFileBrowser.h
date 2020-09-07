#ifndef IMGUIFILEBROWSER_H
#define IMGUIFILEBROWSER_H

#include "imgui.h"
#include <string>
#include <vector>


enum FileBrowser_ActionID
{
	ID_SELECT_FILE,
	ID_SAVE_FILE,
	ID_SELECT_DIR,
	ID_CRT_FILE,
	ID_CRT_DIR
};

namespace imgui_addons
{
    class ImGuiFileBrowser
    {
        public:
            ImGuiFileBrowser();
            ~ImGuiFileBrowser();
			bool showFileDialog( std::string label, ImVec2 sz_xy, bool * state, std::string filter_files = "", FileBrowser_ActionID id = ID_SELECT_FILE );
            std::string selected_fn;

			std::string selected_dir;
			std::string selected_file;
			std::string crt_dir_name;
			std::string crt_file_name;

			void reset_Data();

        private:

            struct Info
            {
                Info(std::string name, bool is_hidden) : name(name), is_hidden(is_hidden)
                {
                }
                std::string name;
                bool is_hidden;
            };
            static std::string wStringToString(const wchar_t* wchar_arr);
            static bool alphaSortComparator(const Info& a, const Info& b);

            bool validateFile();
            bool readDIR(std::string path);
            bool onNavigationButtonClick(int idx);
            bool onDirClick(int idx, bool show_drives);
            #if defined (WIN32) || defined (_WIN32) || defined (__WIN32)
            bool loadWindowsDrives(); // Windows Exclusive
            #endif

            void parsePathTabs(std::string str);
            void showErrorModal();
            void showInvalidFileModal();
			void showInvalidFileCreationModal();
            void clearOldEntries();


			char crt_obj_name[100];
            ImGuiTextFilter filter;
            std::string filter_files;
            std::vector<std::string> valid_exts;
            std::vector<std::string> current_dirlist;
            std::vector<Info> subdirs;
            std::vector<Info> subfiles;
            std::vector<Info*> filtered_dirs; // Note: We don't need to call delete. It's just for storing filtered items from subdirs and subfiles so we don't use PassFilter every frame.
            std::vector<Info*> filtered_files;
            std::string current_path;
            int col_items_limit, selected_idx;
            float col_width;
            bool show_hidden, is_dir, filter_dirty;
			FileBrowser_ActionID fb_id;
    };
}


#endif // IMGUIFILEBROWSER_H
