#pragma once

#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include <IconFontCppHeaders/IconsForkAwesome.h>
#include <IconFontCppHeaders/IconsKenney.h>
#include <IconFontCppHeaders/IconsMaterialDesign.h>
#include <IconFontCppHeaders/IconsMaterialDesignIcons.h>
#include <unordered_map>
#include <string>
#include <iostream>
#include <fstream>

#define HI(v)   ImVec4(0.502f, 0.075f, 0.256f, v)
#define MED(v)  ImVec4(0.455f, 0.198f, 0.301f, v)
#define LOW(v)  ImVec4(0.232f, 0.201f, 0.271f, v)
// backgrounds (@todo: complete with BG_MED, BG_LOW)
#define BG(v)   ImVec4(0.200f, 0.220f, 0.270f, v)
// text
#define TEXT(v) ImVec4(0.860f, 0.930f, 0.890f, v)



enum guiStyle
{
	DEFAULT,
	EASY_THEME,
	CLASSIC_DARK,
	DARK,
	DARK2,
	EXTASY,
	MONOCHROME,
	B_STYLE_DARK,
	EXP_STYLE,
	EXP_STYLE_2,
	DRACULA,
	UE4,
	CHERRY,
	DARK_GRAY,
	DARK_GRAY_TOOLS,
	DARK_GRAY_LEFT,
	PHOTOSHOP,
	MAIN_MENU_BAR
};


enum guiFont
{
	DEFAULT_FONT,
	RUDA,
	ROBOTO,
	DROID_SANS,
	PROGGY_TINY,
	ICONS
};

enum Icon_Font
{
	FK,FA,MD,MDI
};


namespace guiStyleManager 
{
	

	struct StyleInstance
	{
		ImVec4 colors[ImGuiCol_COUNT];
		ImVec2 sizes[20];

		
		// colors
		ImVec4 Text = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 TextDisabled = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 WindowBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 ChildBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 PopupBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 Border = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 BorderShadow = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 FrameBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 FrameBgHovered = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 FrameBgActive = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 TitleBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 TitleBgActive = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 TitleBgCollapsed = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 MenuBarBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 ScrollBarBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 ScrollBarGrab = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 ScrollBarGrabHovered = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 ScrollBardGrabActive = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 CheckMark = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 SliderGrab = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 SliderGrabActive = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 Button = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 ButtonHovered = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 ButtonActive = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 Header = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 HeaderHovered = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 HeaderActive = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 Separator = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 SeparatorHovered = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 SeparatorActive = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 ResizeGrip = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 ResizeGripHovered = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 ResizeGripActive = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 Tab = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 TabHovered = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 TabActive = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 TabUnfocused = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 TabUnfocusedActive = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 PlotLines = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 PlotLinesHovered = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 PlotHistogram = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 PlotHistogramHovered = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 TextSelectedBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 ModalWindowDarkening = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 DragDropTarget = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 NavHighlight = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 NavWindowHighlight= ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 ModalWindowDimBg = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

		



		// sizes
		ImVec2 windowPadding, 
			FramePadding, 
			ItemSpacing, 
			ItemInnerSpacing, 
			TouchExtraPadding, 
			WindowTitleAlign, 
			DisplaySafeAreaPadding;
		float IndentSpacing, 
			ScrollbarSize, 
			GrabMinSize, 
			WindowBorderSize, 
			ChildBorderSize, 
			PopupBorderSize, 
			FrameBorderSize, 
			WindowRounding, 
			ChildRounding, 
			PopupRounding, 
			FrameRounding, 
			ScrollbarRounding, 
			GrabRounding;

		ImVec4& operator[](int index)
		{
			
			switch (index)
			{
			case 0: return Text;
			case 1: return TextDisabled;
			case 2: return WindowBg;
			case 3: return ChildBg;
			case 4: return PopupBg;
			case 5: return Border;
			case 6: return BorderShadow;
			case 7: return FrameBg;
			case 8: return FrameBgHovered;
			case 9: return FrameBgActive;
			case 10: return TitleBg;
			case 11: return TitleBgActive;
			case 12: return TitleBgCollapsed;
			case 13: return MenuBarBg;
			case 14: return ScrollBarBg;
			case 15: return ScrollBarGrab;
			case 16: return ScrollBarGrabHovered;
			case 17: return ScrollBardGrabActive;
			case 18: return CheckMark;
			case 19: return SliderGrab;
			case 20: return SliderGrabActive;
			case 21: return Button;
			case 22: return ButtonHovered;
			case 23: return ButtonActive;
			case 24: return Header;
			case 25: return HeaderHovered;
			case 26: return HeaderActive;
			case 27: return Separator;
			case 28: return SeparatorHovered;
			case 29: return SeparatorActive;
			case 30: return ResizeGrip;
			case 31: return ResizeGripHovered;
			case 32: return ResizeGripActive;
			case 33: return Tab;
			case 34: return TabHovered;
			case 35: return TabActive;
			case 36: return TabUnfocused;
			case 37: return TabUnfocusedActive;
			case 38: return PlotLines;
			case 39: return PlotLinesHovered;
			case 40: return PlotHistogram;
			case 41: return PlotHistogramHovered;
			case 42: return TextSelectedBg;
			case 43: return ModalWindowDarkening;
			case 44: return DragDropTarget;
			case 45: return NavHighlight;
			case 46: return NavWindowHighlight;
			case 47: return ModalWindowDimBg;
			}
		}
		void * getSizeVar(int index)
		{
			switch (index)
			{
			case 0:
				return &windowPadding;
			case 1:
				return &FramePadding;
			case 2:
				return &ItemSpacing;
			case 3:
				return &ItemInnerSpacing;
			case 4:
				return &TouchExtraPadding;
			case 5:
				return &WindowTitleAlign;
			case 6:
				return &DisplaySafeAreaPadding;
			case 7:
				return &IndentSpacing;
			case 8:
				return &ScrollbarSize;
			case 9:
				return &GrabMinSize;
			case 10:
				return &WindowBorderSize;
			case 11:
				return &ChildBorderSize;
			case 12:
				return &PopupBorderSize;
			case 13:
				return &FrameBorderSize;
			case 14:
				return &WindowRounding;
			case 15:
				return &ChildRounding;
			case 16:
				return &PopupRounding;
			case 17:
				return &FrameRounding;
			case 18:
				return &ScrollbarRounding;
			case 19:
				return &GrabRounding;
			}
		}
		template <class T> 
		void setSizeVar(T a, int index)
		{
			////std::cout << "setSizeVar(" << index << ")" << std::endl;
			ImVec2 v;
			float f;
			if (index < 7)
				v = (ImVec2&)a;
			else
				f = (float&)a;

			switch (index)
			{

			case 0: windowPadding = v;break; 
			case 1: FramePadding = v;break;
			case 2: ItemSpacing = v;break; 
			case 3: ItemInnerSpacing = v;break;
			case 4:TouchExtraPadding = v;break;
			case 5: WindowTitleAlign = v;break; 
			case 6:DisplaySafeAreaPadding = v;break;
			case 7: IndentSpacing = f; break; 
			case 8:ScrollbarSize = f;break; 
			case 9: GrabMinSize = f;break; 
			case 10: WindowBorderSize = f; break; 
			case 11:ChildBorderSize = f;break;
			case 12:PopupBorderSize = f;break; 
			case 13:FrameBorderSize = f;break; 
			case 14:WindowRounding = f; break; 
			case 15:ChildRounding = f;break; 
			case 16:PopupRounding = f; break;
			case 17:FrameRounding = f;break;
			case 18:ScrollbarRounding = f;break;
			case 19:GrabRounding = f;break;
			 
			}
		}
	};

	struct ImVec3
	{
		float x, y, z = 0.0f;
		ImVec3() {};
		ImVec3(float x, float y, float z):x(x),y(y),z(z)
		{

		}

	};

	static std::unordered_map<std::string, ImFont *> fonts;
	static std::unordered_map<std::string, StyleInstance> styleInstances;

	void LoadFonts()
	{

		ImGuiIO& io = ImGui::GetIO();

		ImFontConfig config;
		config.MergeMode = true;
		ImWchar fa_ranges[]  = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		ImWchar fk_ranges[]  = { ICON_MIN_FK , ICON_MAX_FK , 0 };
		ImWchar md_ranges[]  = { ICON_MIN_MD , ICON_MAX_MD , 0 };
		ImWchar mdi_ranges[] = { ICON_MIN_MDI , ICON_MAX_MDI , 0 };

		std::vector<ImWchar *> ranges;
		ranges.resize(4);
		ranges[0] = fa_ranges;
		ranges[1] = fk_ranges;
		ranges[2] = md_ranges;
		ranges[3] = mdi_ranges;

		ImFont* default_font = io.Fonts->AddFontDefault();
		fonts["default_font"] = default_font;


		// load fonts
		{
			float pSize;
			std::string fontFamily;
			std::string fontChild;
			std::string font_label;
			std::string fontPath = "../Data/Fonts/";
			std::string filePath;
			// RUDA //
			{
				ImFont* ruda_regular8 = io.Fonts->AddFontFromFileTTF("../Data/Fonts/Ruda/Ruda-Regular.ttf", 8.0f);
				fonts["ruda_regular_8"] = ruda_regular8;

				ImFont* ruda_regular12 = io.Fonts->AddFontFromFileTTF("../Data/Fonts/Ruda/Ruda-Regular.ttf", 12.0f);
				fonts["ruda_regular_12"] = ruda_regular12;

				ImFont* ruda_regular16 = io.Fonts->AddFontFromFileTTF("../Data/Fonts/Ruda/Ruda-Regular.ttf", 16.0f);
				fonts["ruda_regular_16"] = ruda_regular16;

				ImFont* ruda_regular24 = io.Fonts->AddFontFromFileTTF("../Data/Fonts/Ruda/Ruda-Regular.ttf", 24.0f);
				fonts["ruda_regular_24"] = ruda_regular24;


				//
				ImFont* ruda_bold8 = io.Fonts->AddFontFromFileTTF("../Data/Fonts/Ruda/Ruda-Bold.ttf", 8.0f);
				fonts["ruda_bold_8"] = ruda_bold8;

				ImFont* ruda_bold12 = io.Fonts->AddFontFromFileTTF("../Data/Fonts/Ruda/Ruda-Bold.ttf", 12.0f);
				fonts["ruda_bold_12"] = ruda_bold12;

				ImFont* ruda_bold16 = io.Fonts->AddFontFromFileTTF("../Data/Fonts/Ruda/Ruda-Bold.ttf", 16.0f);
				fonts["ruda_bold_16"] = ruda_bold16;

				ImFont* ruda_bold24 = io.Fonts->AddFontFromFileTTF("../Data/Fonts/Ruda/Ruda-Bold.ttf", 24.0f);
				fonts["ruda_bold_24"] = ruda_bold24;


			}



			// ROBOTO //
			{


				// 12
				pSize = 12.0f;
				fontFamily = "Roboto";
				fontChild = "Regular";
				fontPath += "Roboto/";
				font_label = fontFamily + "_" + fontChild + "_" + std::to_string(static_cast<int>(pSize));
				filePath = fontPath + fontFamily + "-" + fontChild + ".ttf";


				ImFont* roboto12 = io.Fonts->AddFontFromFileTTF(filePath.c_str(), pSize);
				fonts[font_label.c_str()] = roboto12;
				fonts["icons_default_12"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fontawesome-webfont.ttf", pSize, &config, ranges[0]);
				fonts["icons_regular_12"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fa-regular-400.ttf", pSize, &config, ranges[0]);
				fonts["icons_solid_12"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fa-solid-900.ttf", pSize, &config, ranges[0]);
				fonts["roboto_regular_12_MDI"] = io.Fonts->AddFontFromFileTTF(filePath.c_str(), pSize);
				fonts["icons_mdi_12"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/materialdesignicons-webfont.ttf", pSize, &config, ranges[3]);
				fonts["roboto_regular_12_MD"] = io.Fonts->AddFontFromFileTTF(filePath.c_str(), pSize);
				fonts["icons_md_12"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/MaterialIcons-Regular.ttf", pSize, &config, ranges[2]);

				// 14
				pSize = 14.0f;
				font_label = fontFamily + "_" + fontChild + "_" + std::to_string(static_cast<int>(pSize));
				ImFont* roboto14 = io.Fonts->AddFontFromFileTTF(filePath.c_str(), pSize);
				fonts[font_label.c_str()] = roboto14;
				fonts["icons_default_14"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fontawesome-webfont.ttf", pSize, &config, ranges[0]);
				fonts["icons_regular_14"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fa-regular-400.ttf", pSize, &config, ranges[0]);
				fonts["icons_solid_14"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fa-solid-900.ttf", pSize, &config, ranges[0]);

				fonts["roboto_regular_14_MDI"] = io.Fonts->AddFontFromFileTTF(filePath.c_str(), pSize);
				fonts["icons_mdi_14"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/materialdesignicons-webfont.ttf", pSize, &config, ranges[3]);

				fonts["roboto_regular_14_MD"] = io.Fonts->AddFontFromFileTTF(filePath.c_str(), pSize);
				fonts["icons_md_14"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/MaterialIcons-Regular.ttf", pSize, &config, ranges[2]);


				// 15
				pSize = 15.0f;
				font_label = fontFamily + "_" + fontChild + "_" + std::to_string(static_cast<int>(pSize));
				ImFont* roboto15 = io.Fonts->AddFontFromFileTTF(filePath.c_str(), pSize);
				fonts[font_label.c_str()] = roboto14;
				fonts["icons_default_15"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fontawesome-webfont.ttf", pSize, &config, ranges[0]);
				fonts["icons_regular_15"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fa-regular-400.ttf", pSize, &config, ranges[0]);
				fonts["icons_solid_15"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fa-solid-900.ttf", pSize, &config, ranges[0]);

				fonts["roboto_regular_15_MDI"] = io.Fonts->AddFontFromFileTTF(filePath.c_str(), pSize);
				fonts["icons_mdi_15"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/materialdesignicons-webfont.ttf", pSize, &config, ranges[3]);

				fonts["roboto_regular_15_MD"] = io.Fonts->AddFontFromFileTTF(filePath.c_str(), pSize);
				fonts["icons_md_15"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/MaterialIcons-Regular.ttf", pSize, &config, ranges[2]);

				// 16
				pSize = 16.0f;
				font_label = fontFamily + "_" + fontChild + "_" + std::to_string(static_cast<int>(pSize));
				ImFont* roboto16 = io.Fonts->AddFontFromFileTTF(filePath.c_str(), pSize);
				fonts[font_label.c_str()] = roboto16;
				fonts["icons_default_16"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fontawesome-webfont.ttf", pSize, &config, ranges[0]);
				fonts["icons_regular_16"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fa-regular-400.ttf", pSize, &config, ranges[0]);
				fonts["icons_solid_16"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fa-solid-900.ttf", pSize, &config, ranges[0]);
				

				fonts["roboto_regular_16_MDI"] = io.Fonts->AddFontFromFileTTF(filePath.c_str(), pSize);
				fonts["icons_mdi_16"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/materialdesignicons-webfont.ttf", pSize, &config, ranges[3]);
				
				fonts["roboto_regular_16_MD"] = io.Fonts->AddFontFromFileTTF(filePath.c_str(), pSize);
				fonts["icons_md_16"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/MaterialIcons-Regular.ttf", pSize, &config, ranges[2]);
				

				// 18
				pSize = 18.0f;
				font_label = fontFamily + "_" + fontChild + "_" + std::to_string(static_cast<int>(pSize));
				ImFont* roboto18 = io.Fonts->AddFontFromFileTTF(filePath.c_str(), pSize);
				fonts[font_label.c_str()] = roboto18;
				fonts["icons_default_18"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fontawesome-webfont.ttf", pSize, &config, ranges[0]);
				fonts["icons_regular_18"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fa-regular-400.ttf", pSize, &config, ranges[0]);
				fonts["icons_solid_18"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fa-solid-900.ttf", pSize, &config, ranges[0]);
				
				fonts["roboto_regular_18_MDI"] = io.Fonts->AddFontFromFileTTF(filePath.c_str(), pSize);
				fonts["icons_mdi_18"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/materialdesignicons-webfont.ttf", pSize, &config, ranges[3]);
				
				fonts["roboto_regular_18_MD"] = io.Fonts->AddFontFromFileTTF(filePath.c_str(), pSize);
				fonts["icons_md_18"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/MaterialIcons-Regular.ttf", pSize, &config, ranges[2]);

				// 20
				pSize = 20.0f;
				font_label = fontFamily + "_" + fontChild + "_" + std::to_string(static_cast<int>(pSize));
				ImFont* roboto20 = io.Fonts->AddFontFromFileTTF(filePath.c_str(), pSize);
				fonts[font_label.c_str()] = roboto20;
				fonts["icons_default_20"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fontawesome-webfont.ttf", pSize, &config, ranges[0]);
				fonts["icons_regular_20"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fa-regular-400.ttf", pSize, &config, ranges[0]);
				fonts["icons_solid_20"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fa-solid-900.ttf", pSize, &config, ranges[0]);
				
				fonts["roboto_regular_20_MDI"] = io.Fonts->AddFontFromFileTTF(filePath.c_str(), pSize);
				fonts["icons_mdi_20"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/materialdesignicons-webfont.ttf", pSize, &config, ranges[3]);
				fonts["roboto_regular_20_MD"] = io.Fonts->AddFontFromFileTTF(filePath.c_str(), pSize);
				fonts["icons_md_20"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/MaterialIcons-Regular.ttf", pSize, &config, ranges[2]);

			
				// bold12
				{pSize = 12.0f;
				fontChild = "Bold";
				filePath = fontPath + fontFamily + "-" + fontChild + ".ttf";
				font_label = fontFamily + "_" + fontChild + "_" + std::to_string(static_cast<int>(pSize));

				ImFont* roboto_bold12 = io.Fonts->AddFontFromFileTTF(filePath.c_str(), pSize);
				fonts[font_label.c_str()] = roboto_bold12;

				
				// bold14
				pSize = 14.0f;
				font_label = fontFamily + "_" + fontChild + "_" + std::to_string(static_cast<int>(pSize));
				ImFont* roboto_bold14 = io.Fonts->AddFontFromFileTTF(filePath.c_str(), pSize);
				fonts[font_label.c_str()] = roboto_bold14;
				fonts["icons_defaultBold_14"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fontawesome-webfont.ttf", pSize, &config, ranges[0]);
				fonts["icons_regularBold_14"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fa-regular-400.ttf", pSize, &config, ranges[0]);
				fonts["icons_solidBold_14"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fa-solid-900.ttf", pSize, &config, ranges[0]);
				

				// bold16
				pSize = 16.0f;
				font_label = fontFamily + "_" + fontChild + "_" + std::to_string(static_cast<int>(pSize));
				ImFont* roboto_bold16 = io.Fonts->AddFontFromFileTTF(filePath.c_str(), pSize);
				fonts[font_label.c_str()] = roboto_bold16;
				fonts["icons_defaultBold_16"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fontawesome-webfont.ttf", pSize, &config, ranges[0]);
				fonts["icons_regularBold_16"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fa-regular-400.ttf", pSize, &config, ranges[0]);
				fonts["icons_solidBold_16"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fa-solid-900.ttf", pSize, &config, ranges[0]);
				

				// bold20
				pSize = 20.0f;
				font_label = fontFamily + "_" + fontChild + "_" + std::to_string(static_cast<int>(pSize));
				ImFont* roboto_bold20 = io.Fonts->AddFontFromFileTTF(filePath.c_str(), pSize);
				fonts[font_label.c_str()] = roboto_bold20;
				}


				// DroidSans
				{
					pSize = 12.0f;
					ImFont* droidSans12 = io.Fonts->AddFontFromFileTTF("../Data/Fonts/DroidSans.ttf", pSize);
					fonts["droidSans_regular_12"] = droidSans12;

					pSize = 14.0f;
					ImFont* droidSans14 = io.Fonts->AddFontFromFileTTF("../Data/Fonts/DroidSans.ttf", pSize);
					fonts["droidSans_regular_14"] = droidSans14;

					pSize = 16.0f;
					ImFont* droidSans16 = io.Fonts->AddFontFromFileTTF("../Data/Fonts/DroidSans.ttf", pSize);
					fonts["droidSans_regular_16"] = droidSans16;

					pSize = 20.0f;
					ImFont* droidSans20 = io.Fonts->AddFontFromFileTTF("../Data/Fonts/DroidSans.ttf", pSize);
					fonts["droidSans_regular_20"] = droidSans20;

				}

				// ProggyTiny
				{
					ImFont* proggyTiny8 = io.Fonts->AddFontFromFileTTF("../Data/Fonts/ProggyTiny.ttf", 8.0f);
					fonts["proggyTiny_regular_8"] = proggyTiny8;

					ImFont* proggyTiny12 = io.Fonts->AddFontFromFileTTF("../Data/Fonts/ProggyTiny.ttf", 12.0f);
					fonts["proggyTiny_regular_12"] = proggyTiny12;

					ImFont* proggyTiny16 = io.Fonts->AddFontFromFileTTF("../Data/Fonts/ProggyTiny.ttf", 16.0f);
					fonts["proggyTiny_regular_16"] = proggyTiny16;

					ImFont* proggyTiny24 = io.Fonts->AddFontFromFileTTF("../Data/Fonts/ProggyTiny.ttf", 24.0f);
					fonts["proggyTiny_regular_24"] = proggyTiny24;
				}


			}
			{

				config.MergeMode = false;
				float pSize = 26.0f;

				io.Fonts->AddFontDefault();
				ImWchar icon_ranges0[] = { ICON_MIN_FA , ICON_MAX_FA, 0 };
				fonts["icons_FA"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/fontawesome-webfont.ttf", pSize, &config, icon_ranges0);
				io.Fonts->Build();


				io.Fonts->AddFontDefault();
				ImWchar icon_ranges2[] = { ICON_MIN_MDI , ICON_MAX_MDI , 0 };
				fonts["icons_MDI"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/materialdesignicons-webfont.ttf", pSize, &config, icon_ranges2);
				io.Fonts->Build();

				
				io.Fonts->AddFontDefault();
				ImWchar icon_ranges3[] = { ICON_MIN_KI , ICON_MAX_KI , 0 };
				fonts["icons_KI"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/kenney-icon-font.ttf", pSize, &config, icon_ranges3);
				io.Fonts->Build();

				io.Fonts->AddFontDefault();
				ImWchar icon_ranges4[] = { ICON_MIN_FK , ICON_MAX_FK , 0 };
				fonts["icons_FK"] = io.Fonts->AddFontFromFileTTF("../Data/Fonts/webfonts/forkawesome-webfont.ttf", pSize, &config, icon_ranges4);
				io.Fonts->Build();
			}
			


		}
	}

	StyleInstance loadStyleFromFile(std::string filepath)
	{


		StyleInstance style_inst;

		std::ifstream file(filepath);
		std::string str;

		std::vector<float> tkns;
		std::string debug_log = "";

		int line_index = 0;
		while (std::getline(file, str) && line_index < 48 + 20)
		{

			debug_log += " - - - - - - - - - - \n";
			debug_log += "\n Line : " + std::to_string(line_index) + "\n";
			tkns.clear();
			char * tokens = strtok((char *)str.c_str(), " ");

			debug_log += "Line tokens : ";
			while (tokens)
			{
				debug_log += " "; debug_log += tokens; debug_log += " | ";
				std::string temp = std::string(tokens);
				tkns.push_back(std::stof(temp));
				tokens = strtok(NULL, " ");
			}
			debug_log += "\n";

			int tokens_length = tkns.size();
			if (tokens_length == 4)
			{
				
				ImVec4& v4 = ImVec4(tkns[0], tkns[1], tkns[2], tkns[3]);
				style_inst[line_index] = v4;
				debug_log += " ImVec4 v : " + std::to_string(v4.x) + " , " + std::to_string(v4.y) + " , " + std::to_string(v4.z) + " , " + std::to_string(v4.w) + "\n";
			}
			else if (tokens_length == 2)
			{
				ImVec2 v2 = ImVec2(tkns[0], tkns[1]);
				style_inst.setSizeVar(v2,line_index - 48);
				debug_log += " ImVec2 v : " + std::to_string(v2.x) + " , " + std::to_string(v2.y) + "\n";

			}
			else if (tokens_length == 1)
			{
				float v = tkns[0];
				style_inst.setSizeVar(v, line_index - 48);
				debug_log += std::to_string(v) + "\n";
			}
			
			line_index++;
			debug_log += "\n\n";

//#define SHOW_DEBUG_LOG
#ifdef SHOW_DEBUG_LOG
			//std::cout << debug_log << std::endl;
			system("pause");
#endif
			debug_log = "";
			
		}


		
//#define SHOW_DEBUG_LOG_2
#ifdef SHOW_DEBUG_LOG_2
		//std::cout << std::endl;
		for (int i = 7; i < 20; i++)
		{
			//std::cout << *(float *)styleInstances["main_style"].getSizeVar(i) << std::endl;
		}
#endif


		return style_inst;
	}



	void LoadStyles()
	{
		
		styleInstances["main_style"]        = loadStyleFromFile("../Data/Styles/main_style.txt");
		styleInstances["main_style_bot_panel"] = loadStyleFromFile("../Data/Styles/main_style_bot_panel.txt");
		styleInstances["main_style"].ModalWindowDarkening = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

		styleInstances["main_style2"]       = loadStyleFromFile("../Data/Styles/main_style2.txt");
		styleInstances["main_style_bottom"] = loadStyleFromFile("../Data/Styles/main_style_bottom.txt");
		styleInstances["child_style"]       = loadStyleFromFile("../Data/Styles/child_style.txt");
		styleInstances["child_style2"]      = loadStyleFromFile("../Data/Styles/child_style2.txt");
		styleInstances["menu_bar_style"]    = loadStyleFromFile("../Data/Styles/menu_bar_style.txt");
		styleInstances["main_menu_bar_style"] = loadStyleFromFile("../Data/Styles/main_menu_bar_style.txt");
		styleInstances["tools_style"]       = loadStyleFromFile("../Data/Styles/tools_style.txt");
		styleInstances["tools_style2"]      = loadStyleFromFile("../Data/Styles/tools_style2.txt");
		styleInstances["tools_helpers"]     = loadStyleFromFile("../Data/Styles/tools_helpers.txt");

		styleInstances["main_style_bottom"] = loadStyleFromFile("../Data/Styles/main_style_bottom.txt");
		styleInstances["main_style_right"]  = loadStyleFromFile("../Data/Styles/main_style_right.txt");
		styleInstances["main_style_right2"] = loadStyleFromFile("../Data/Styles/main_style_right2.txt");
		styleInstances["main_style_child1"] = loadStyleFromFile("../Data/Styles/main_style_child1.txt");
		styleInstances["main_style_child2"] = loadStyleFromFile("../Data/Styles/main_style_child2.txt");

		styleInstances["main_style_right_child00"] = loadStyleFromFile("../Data/Styles/main_style_right_child00.txt");
		styleInstances["main_style_right_child0"] = loadStyleFromFile("../Data/Styles/main_style_right_child0.txt");
		styleInstances["main_style_right_child1"] = loadStyleFromFile("../Data/Styles/main_style_right_child1.txt");
		styleInstances["main_style_right_child2"] = loadStyleFromFile("../Data/Styles/main_style_right_child2.txt");
		styleInstances["main_style_right_child3"] = loadStyleFromFile("../Data/Styles/main_style_right_child3.txt");
		styleInstances["main_style_right_child3_list"] = loadStyleFromFile("../Data/Styles/main_style_right_child3_list.txt");

		styleInstances["main_style_right_child_transformations"] = loadStyleFromFile("../Data/Styles/main_style_right_child_transformations.txt");
		styleInstances["main_style_right_child_transformations_parent"] = loadStyleFromFile("../Data/Styles/main_style_right_child_transformations_parent.txt");
		styleInstances["main_style_right_child_transformations_inner1"] = loadStyleFromFile("../Data/Styles/main_style_right_child_transformations_inner1.txt");
		styleInstances["main_style_right_child_transformations2"] = loadStyleFromFile("../Data/Styles/main_style_right_child_transformations2.txt");
		styleInstances["main_style_right_child_transformations3"] = loadStyleFromFile("../Data/Styles/main_style_right_child_transformations3.txt");
		styleInstances["main_style_right_child_transformations22"] = loadStyleFromFile("../Data/Styles/main_style_right_child_transformations22.txt");
		styleInstances["viewports_frame_style"] = loadStyleFromFile("../Data/Styles/viewports_frame_style.txt");
		styleInstances["viewports_frame_style2"] = loadStyleFromFile("../Data/Styles/viewports_frame_style2.txt");
		styleInstances["viewports_popup_style"] = loadStyleFromFile("../Data/Styles/viewports_popup_style.txt");
		styleInstances["viewports_popup_style2"] = loadStyleFromFile("../Data/Styles/viewports_popup_style2.txt");
		styleInstances["main_style_leftPanel"] = loadStyleFromFile("../Data/Styles/main_style_leftPanel.txt");
		styleInstances["style_import_popup"] = loadStyleFromFile("../Data/Styles/style_import_popup.txt");
		styleInstances["style_import_popup_2"] = loadStyleFromFile("../Data/Styles/style_import_popup_2.txt");
		styleInstances["style_import_popup_2_inner"] = loadStyleFromFile("../Data/Styles/style_import_popup_2_inner.txt");
		styleInstances["style_popup"] = loadStyleFromFile("../Data/Styles/style_popup.txt");
		styleInstances["style_popup2"] = loadStyleFromFile("../Data/Styles/style_popup2.txt");
		styleInstances["style_popup_group"] = loadStyleFromFile("../Data/Styles/style_popup_group.txt");
		styleInstances["style_popup_dir"] = loadStyleFromFile("../Data/Styles/style_popup_dir.txt");
		styleInstances["inner_transformation_child"] = loadStyleFromFile("../Data/Styles/inner_transformation_child.txt");

		
		styleInstances["style_restrictions_list"] = loadStyleFromFile("../Data/Styles/style_restrictions_list.txt");
		styleInstances["style_restrictions_panel"] = loadStyleFromFile("../Data/Styles/style_restrictions_panel.txt");
		styleInstances["style_restrictions_panel2"] = loadStyleFromFile("../Data/Styles/style_restrictions_panel2.txt");
		styleInstances["style_restrictions_params"] = loadStyleFromFile("../Data/Styles/style_restrictions_params.txt");
		
		return;

		StyleInstance style_inst;
	
		// create Main app style 3
		{
			float f = 1 / 255.0f;

			// colors
			{

				style_inst[0] = ImVec4(102 * f, 102 * f, 102 * f, 255 * f);
				style_inst[1] = ImVec4(72 *f,72*f,72*f,255*f); // style3
				style_inst[2] = ImVec4(93 * f, 92 * f, 88 * f, 255 * f); // style3
				style_inst[3] = ImVec4(59 * f, 51 * f, 44 * f, 255 * f);    //  
				style_inst[4] = ImVec4(43 * f, 30 * f, 22 * f, 180 * f); //style3
				style_inst[5] = ImVec4(121 *f , 144*f,140*f , 49*f); // style3
				style_inst[6] = ImVec4(60 * f, 60 * f, 60 * f, 255 * f);
				style_inst[7] = ImVec4(44 * f, 44 * f, 43 * f, 255 * f);
				style_inst[8] = ImVec4(33 * f, 33 * f, 33 * f, 254 * f);
				style_inst[9] = ImVec4(48 * f, 48 * f, 48 * f, 255 * f);
				style_inst[10] = ImVec4(56 * f, 56 * f, 56 * f, 255 * f);
				style_inst[11] = ImVec4(43 * f, 43 * f, 43 * f, 230 * f);
				style_inst[12] = ImVec4(70 * f, 67 * f, 59 * f, 255 * f);
				style_inst[13] = ImVec4(75 * f, 68 * f, 65 * f, 255 * f);
				style_inst[14] = ImVec4(101 * f, 99 * f, 92 * f, 255 * f);
				style_inst[15] = ImVec4(137 * f, 133 * f, 124 * f, 253 * f);
				style_inst[16] = ImVec4(194 * f, 194 * f, 194 * f, 255 * f);
				style_inst[17] = ImVec4(139 * f, 127 * f, 127 * f, 255 * f);
				style_inst[18] = ImVec4(173 * f, 97 * f, 96 * f, 255 * f);
				style_inst[19] = ImVec4(163 * f, 163 * f, 163 * f, 255 * f);
				style_inst[20] = ImVec4(255 * f, 0 * f, 0 * f, 255 * f);
				style_inst[21] = ImVec4(59 * f, 59 * f, 59 * f, 255 * f);
				style_inst[22] = ImVec4(46 * f, 46 * f, 46 * f, 255 * f);
				style_inst[23] = ImVec4(72 * f, 72 * f, 72 * f, 255 * f); // style3
				style_inst[24] = ImVec4(60 * f, 60 * f, 60 * f, 255 * f); // style3
				style_inst[25] = ImVec4(56 * f, 56 * f, 56 * f, 255 * f); // style3
				style_inst[26] = ImVec4(0 * f, 0 * f, 0 * f, 66 * f);
				style_inst[27] = ImVec4(179 * f, 171 * f, 153 * f, 74 * f);
				style_inst[28] = ImVec4(179 * f, 171 * f, 153 * f, 172 * f);
				style_inst[29] = ImVec4(62 * f, 103 * f, 114 * f, 255 * f);
				style_inst[30] = ImVec4(66 * f, 150 * f, 250 * f, 171 * f);
				style_inst[31] = ImVec4(66 * f, 150 * f, 250 * f, 242 * f);
				style_inst[32] = ImVec4(184 * f, 52 * f, 52 * f, 128 * f);
				style_inst[33] = ImVec4(119 * f, 25 * f, 25 * f, 153 * f);
				style_inst[34] = ImVec4(58 * f, 8 * f, 8 * f, 255 * f);
				style_inst[35] = ImVec4(156 * f, 156 * f, 156 * f, 255 * f);
				style_inst[36] = ImVec4(255 * f, 110 * f, 89 * f, 255 * f);
				style_inst[37] = ImVec4(230 * f, 179 * f, 0 * f, 255 * f);
				style_inst[38] = ImVec4(255 * f, 153 * f, 0 * f, 255 * f);
				style_inst[39] = ImVec4(186 * f, 186 * f, 186 * f, 89 * f);
				style_inst[40] = ImVec4(28 * f, 119 * f, 132 * f, 89 * f);
				style_inst[41] = ImVec4(255 * f, 255 * f, 0 * f, 230 * f);
				style_inst[42] = ImVec4(66 * f, 150 * f, 250 * f, 255 * f);
				style_inst[43] = ImVec4(255 * f, 255 * f, 255 * f, 179 * f);
				style_inst[44] = ImVec4(255 * f, 255 * f, 255 * f, 179 * f);
				style_inst[45] = ImVec4(255 * f, 255 * f, 255 * f, 179 * f);
				style_inst[46] = ImVec4(255 * f, 255 * f, 255 * f, 179 * f);
				style_inst[47] = ImVec4(255 * f, 255 * f, 255 * f, 179 * f);
				//style_inst[44] = ImVec4(255 * f, 255 * f, 255 * f, 179 * f);
			}

			// sizes
			{
				style_inst.windowPadding = ImVec2(10, 2);
				style_inst.FramePadding = ImVec2(4, 5);
				style_inst.ItemSpacing = ImVec2(10, 2);
				style_inst.ItemInnerSpacing = ImVec2(5, 5);
				style_inst.TouchExtraPadding = ImVec2(0, 0);
				style_inst.IndentSpacing = 15;
				style_inst.ScrollbarSize = 10;
				style_inst.GrabMinSize = 10;
				style_inst.WindowBorderSize = 1;
				style_inst.ChildBorderSize = 1;
				style_inst.PopupBorderSize = 1;
				style_inst.FrameBorderSize = 1;
				style_inst.WindowRounding = 5;
				style_inst.FrameRounding = 5;
				style_inst.ChildRounding = 5;
				style_inst.PopupRounding = 5;
				style_inst.ScrollbarRounding = 3;
				style_inst.GrabRounding = 4;
				style_inst.WindowTitleAlign = ImVec2(0.02, 0.5);
				style_inst.DisplaySafeAreaPadding = ImVec2(19, 14);
			}

			styleInstances["main_style"] = style_inst;
		}

	}

	void setIconFont(const std::string font)
	{
		ImGui::PushFont(fonts[font]);
	}

	void setIconFont(const Icon_Font key , int size = 16)
	{
		// roboto_regular_14_MDI
		std::string font_id = "roboto_regular";
		std::string _size = std::to_string(size);
		switch (key)
		{
		case FA: font_id  += "_" + _size + "_FA";
		case FK: font_id  += "_" + _size + "_FK";
		case MD: font_id  += "_" + _size + "_MD";
		case MDI: font_id += "_" + _size + "_MDI";
		}
		
		ImGui::PushFont(fonts[font_id]);
	}

	void setFont(int id = 0 , const std::string child = "Regular" , int pSize = 16)
	{

		std::string fontFamily;

		switch (id)
		{
		case DEFAULT_FONT:
		{
			fontFamily = "default_font";
		}break;

		case RUDA:
		{
			fontFamily = "ruda";
		}break;

		case ROBOTO:
		{
			fontFamily = "Roboto";
		}break;

		case DROID_SANS:
		{
			fontFamily = "droidSans";
		}break;

		case PROGGY_TINY:
		{
			fontFamily = "proggyTiny";
		}break;

		case ICONS:
		{
			fontFamily = "icons";
		}break;

		}

		std::string key = fontFamily + "_" + child + "_" + std::to_string(pSize);

		auto it = fonts.find(key);
		if (it != fonts.end())
		{
			ImGui::PushFont(it->second);
		}
		else
		{
			ImGui::PushFont(fonts["default_font"]);
		}


	}

	void setStyle(int id, bool darkStyle = false, float alpha_factor = 1.0f)
	{
		
		
	}

	void setStyle(std::string style_id)
	{
		
		ImGuiStyle& style = ImGui::GetStyle();
		StyleInstance inst = styleInstances[style_id.c_str()];
		
		// colors
		{
			for (int i = 0; i < ImGuiCol_COUNT; i++)
			{
				style.Colors[i] = inst[i];
			}

			/*
			style.Colors[ImGuiCol_Text] = inst[0];
			style.Colors[ImGuiCol_TextDisabled] = inst[1];
			style.Colors[ImGuiCol_WindowBg] = inst[2];
			style.Colors[ImGuiCol_ChildBg] = inst[3];
			style.Colors[ImGuiCol_PopupBg] = inst[4];
			style.Colors[ImGuiCol_Border] = inst[5]; 
			style.Colors[ImGuiCol_BorderShadow] = inst[6];
			style.Colors[ImGuiCol_FrameBg] = inst[7];
			style.Colors[ImGuiCol_FrameBgHovered] = inst[8];
			style.Colors[ImGuiCol_FrameBgActive] = inst[9];
			style.Colors[ImGuiCol_TitleBg] = inst[10];
			style.Colors[ImGuiCol_TitleBgActive] = inst[11];
			style.Colors[ImGuiCol_TitleBgCollapsed] = inst[12];
			style.Colors[ImGuiCol_MenuBarBg] = inst[13];
			style.Colors[ImGuiCol_ScrollbarBg] = inst[13];
			style.Colors[ImGuiCol_ScrollbarGrab] = inst[14];
			style.Colors[ImGuiCol_ScrollbarGrabHovered] = inst[15];
			style.Colors[ImGuiCol_ScrollbarGrabActive] = inst[16];
			style.Colors[ImGuiCol_CheckMark] = inst[17];
			style.Colors[ImGuiCol_SliderGrab] = inst[18];
			style.Colors[ImGuiCol_SliderGrabActive] = inst[19];
			style.Colors[ImGuiCol_Button] = inst[20];
			style.Colors[ImGuiCol_ButtonHovered] = inst[21];
			style.Colors[ImGuiCol_ButtonActive] = inst[22];
			style.Colors[ImGuiCol_Header] = inst[23];
			style.Colors[ImGuiCol_HeaderHovered] = inst[24];
			style.Colors[ImGuiCol_HeaderActive] = inst[25];
			style.Colors[ImGuiCol_Separator] = inst[26];
			style.Colors[ImGuiCol_SeparatorHovered] = inst[27];
			style.Colors[ImGuiCol_SeparatorActive] = inst[28];
			style.Colors[ImGuiCol_ResizeGrip] = inst[29];
			style.Colors[ImGuiCol_ResizeGripHovered] = inst[30];
			style.Colors[ImGuiCol_ResizeGripActive] = inst[31];
			style.Colors[ImGuiCol_Tab] = inst[32];
			style.Colors[ImGuiCol_TabHovered] = inst[33];
			style.Colors[ImGuiCol_TabActive] = inst[34];
			style.Colors[ImGuiCol_TabUnfocused] = inst[35];
			style.Colors[ImGuiCol_TabUnfocusedActive] = inst[36];
			style.Colors[ImGuiCol_PlotLines] = inst[37];
			style.Colors[ImGuiCol_PlotLinesHovered] = inst[36];
			style.Colors[ImGuiCol_PlotHistogram] = inst[37];
			style.Colors[ImGuiCol_PlotHistogramHovered] = inst[38];
			style.Colors[ImGuiCol_TextSelectedBg] = inst[39];
			style.Colors[ImGuiCol_ModalWindowDarkening] = inst[40];
			style.Colors[ImGuiCol_DragDropTarget] = inst[41];
			style.Colors[ImGuiCol_NavHighlight] = inst[42];
			style.Colors[ImGuiCol_NavWindowingHighlight] = inst[43];
			style.Colors[ImGuiCol_ModalWindowDimBg] = inst[44];
			*/
		}

		// sizes
		{
			style.WindowPadding = inst.windowPadding;
			style.FramePadding = inst.FramePadding;
			style.ItemSpacing = inst.ItemSpacing;
			style.ItemInnerSpacing = inst.ItemInnerSpacing;
			style.TouchExtraPadding = inst.TouchExtraPadding;
			style.IndentSpacing = inst.IndentSpacing;
			style.ScrollbarSize = inst.ScrollbarSize;
			style.GrabMinSize = inst.GrabMinSize;
			style.WindowBorderSize = inst.WindowBorderSize;
			style.ChildBorderSize = inst.ChildBorderSize;
			style.FrameBorderSize = inst.FrameBorderSize;
			style.PopupBorderSize = inst.PopupBorderSize;
			style.WindowRounding = inst.WindowRounding;
			style.ChildRounding = inst.ChildRounding;
			style.FrameRounding = inst.FrameRounding;
			style.PopupRounding = inst.PopupRounding;
			style.ScrollbarRounding = inst.ScrollbarRounding;
			style.GrabRounding = inst.GrabRounding;
			style.WindowTitleAlign = inst.WindowTitleAlign;
			style.DisplaySafeAreaPadding = inst.DisplaySafeAreaPadding;
		}
		
	}



	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
	//
	//
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 


	template <class T>
	void printStyleVarToFile(T& obj, std::ofstream *file, std::string label)
	{
		std::string class_name = typeid(T).name();
		////std::cout << class_name << std::endl;

		int case_id = -1;
		if (class_name == "struct ImVec2") case_id = 0;
		if (class_name == "struct ImVec3") case_id = 1;
		if (class_name == "struct ImVec4") case_id = 2;
		if (class_name == "float") case_id = 3;
		if (class_name == "int") case_id = 4;

		ImVec2& v2 = ImVec2(0.0f,0.0f);
		ImVec3& v3 = ImVec3(0.0f,0.0f,0.0f);
		ImVec4& v4 = ImVec4(0.0f,0.0f,0.0f,0.0f);

		switch (case_id)
		{

		case 0:
			////std::cout << "ImVec2" << std::endl;
			v2 = (ImVec2&)obj;
			*file << v2.x << " " << v2.y << "\n";
			break;
		case 1:
			////std::cout << "ImVec3" << std::endl;
			v3 = (ImVec3&)obj;
			*file << v3.x << " " << v3.y << " " << v3.z << "\n";
			break;
		case 2:
			////std::cout << "ImVec4" << std::endl;
			v4 = (ImVec4&)obj;
			*file << v4.x << " " << v4.y << " " << v4.z << " " << v4.w << "\n";
			break;
		case 3:
			////std::cout << "float" << std::endl;
			*file << (float&)obj << "\n";
			break;
		case 4:
			////std::cout << "int" << std::endl;
			*file << (int&)obj << "\n";
			break;

		}
	}
	
	void exportStyleToFile(std::string style_id)
	{
		
		std::ofstream file;
		file.open("../Data/Styles/"+style_id+".txt");
		

		//file << "colors\n";
		int color_vars_size = 48;
		for (int i = 0; i < color_vars_size; i++)
		{
			//ImVec4 v = styleInstances["main_style_3"][i];
			////std::cout << " v[" << i << "] : ( " << v.x << " , " << v.y << " , " << v.z << " , " << v.w << " ) " << std::endl;

			printStyleVarToFile(styleInstances[ style_id.c_str() ][i], &file, "");
		}

		//file << "sizes\n";
		int size_vars_size = 20;
		ImVec2 v2;
		float f;
		for (int i = 0; i < size_vars_size; i++)
		{
			
			if (i < 7)
			{
				v2 = *((ImVec2*)styleInstances[ style_id.c_str() ].getSizeVar(i));
				printStyleVarToFile(v2, &file, "");
			}
			else
			{
				f = *((float*)styleInstances[ style_id.c_str() ].getSizeVar(i));
				printStyleVarToFile(f, &file, "");
			}
			
		}

		file.close();

	}

	void cleanUp()
	{
		
		exportStyleToFile("main_style");
		exportStyleToFile("main_style_bot_panel");
		exportStyleToFile("main_style2");
		exportStyleToFile("main_style_bottom");
		exportStyleToFile("main_style_right");
		exportStyleToFile("main_style_right2");
		exportStyleToFile("main_style_child1");
		exportStyleToFile("main_style_child2");
		exportStyleToFile("main_style_right_child00");
		exportStyleToFile("main_style_right_child0");
		exportStyleToFile("main_style_right_child1");
		exportStyleToFile("main_style_right_child2");
		exportStyleToFile("main_style_right_child3");
		exportStyleToFile("main_style_right_child3_list");

		exportStyleToFile("main_style_right_child_transformations");
		exportStyleToFile("main_style_right_child_transformations_parent");
		exportStyleToFile("main_style_right_child_transformations_inner1");
		exportStyleToFile("main_style_right_child_transformations2");
		exportStyleToFile("main_style_right_child_transformations3");
		exportStyleToFile("main_style_right_child_transformations22");
		exportStyleToFile("child_style");
		exportStyleToFile("child_style2");
		exportStyleToFile("menu_bar_style");
		exportStyleToFile("main_menu_bar_style");
		exportStyleToFile("tools_style");
		exportStyleToFile("tools_style2");
		exportStyleToFile("tools_helpers");
		exportStyleToFile("viewports_frame_style");
		exportStyleToFile("viewports_frame_style2");
		exportStyleToFile("viewports_popup_style");
		exportStyleToFile("viewports_popup_style2");
		exportStyleToFile("main_style_leftPanel");
		exportStyleToFile("style_import_popup");
		exportStyleToFile("style_import_popup_2");
		exportStyleToFile("style_import_popup_2_inner");
		exportStyleToFile("style_popup");
		exportStyleToFile("style_popup2");
		exportStyleToFile("style_popup_group");
		exportStyleToFile("style_popup_dir");
		exportStyleToFile("inner_transformation_child");
		exportStyleToFile("inner_transformation_child");
		exportStyleToFile("style_restrictions_list");
		exportStyleToFile("style_restrictions_panel");
		exportStyleToFile("style_restrictions_panel2");
		exportStyleToFile("style_restrictions_params");
		//"main_style_leftPanel"
		
	}

	
}