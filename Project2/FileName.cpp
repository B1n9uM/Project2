#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <iostream>
#include <filesystem>
#include <ShlObj.h>
#include <cstdlib>
#include <FL/Fl_Native_File_Chooser.H>
#include <vector>
#include <cstdlib>
#include <Windows.h>
#include <system_error>
#include <FL/fl_ask.H>    
#include <FL/Fl_Input.H>
 
//Для удобства 
namespace fs = std::filesystem;
using std::endl;

//Глобальные переменные
std::vector<fs::path> selected_file;
fs::path programmDir;
fs::path targetPath;
Fl_Input* webhook_input = nullptr;
std::error_code ec;

//Выбор файла
void fileSoc(Fl_Widget* widget, void* data) {
    //Настройка окна
    Fl_Native_File_Chooser file;
    file.type(Fl_Native_File_Chooser::BROWSE_MULTI_FILE);
    file.title("Выберите файл");
    file.filter("Все файлы\t*");
    file.directory(".");

    //Получение результата
    switch (file.show()) {
        //Отказ
    case 1:
        std::cout << "Вы отказались выбирать файлы" << endl;
        break;
        //Ошибка
    case -1:
        std::cout << "Ошибка: " << file.errmsg() << endl;
        break;
        //Успех
    default:
        selected_file.reserve(selected_file.size() + file.count());
        for (int i = 0; i < file.count(); i++) {
            std::cout << file.filename(i) << endl;
            selected_file.push_back(file.filename(i));
        }
        break;
    };
}

//Функия для выбора папки 
void dirSoc(Fl_Widget* widget, void* data) {
    //Настрока окна для выбора папки 
    Fl_Native_File_Chooser userDir_soc;
    userDir_soc.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
    userDir_soc.title("Выберите папку куда хотите копировать");
    userDir_soc.filter("Папку\t*");
    userDir_soc.directory(".");

    //Получения результата
    switch (userDir_soc.show()) {
    case 1:
        std::cout << "Вы отказались выберать папку" << endl;
        break;
    case -1:
        std::cout << "Ошибка: " << userDir_soc.errmsg() << endl;
        break;
    default:
        std::cout << "Выбор папки дал true" << endl;
        programmDir = userDir_soc.filename();
    }

}

//Функция для копирования нескольких файлов 
bool copedFiles(const fs::path& programmDir, std::error_code out_ec) {
    bool copedFiles = true;
    out_ec.clear(); 
    //Перебор файлов
    for (const auto file : selected_file) {
        //Успех, файлы копируются 
        targetPath = programmDir / file.filename();
        fs::copy_file(file, targetPath, fs::copy_options::overwrite_existing, out_ec);
        copedFiles = true;
        
        if (out_ec) {
            //Выводит ошибку
            std::cerr << "Ошибка: " << file << out_ec.message() << "\n";
            copedFiles = false;
            break; 
        }
    }
    //Возращение значения
    return copedFiles;
}

//Кнопка для копирования файлов 
void button_copoedFiles(Fl_Widget* widget, void* data) {
    std::error_code local_ec; 
    std::cout << "Кнопка отдала true" << endl;
    //Проверка копирования файлов 
    if (copedFiles(programmDir, local_ec)) {
        std::cout << "Файлы скопировались " << endl; 
    }
    else {
        std::cout << "Ошибка 3. Не удалось скопировать файлы " << endl; 
        std::cerr << "Код ошибки системы: " << local_ec.value() << endl; 
        std::cerr << "Описание ошибки: " << local_ec.message(); 
    }
}


void button_ZipArhive(Fl_Widget* widget, void* data) {

    if (selected_file.empty()) {
        std::cerr << "Ошибка сначала выберите файлы для архивации" << std::endl;
        return;
    }

    fs::path zipPath = programmDir / "archive.zip";
    std::string command = "tar -caf \"" + zipPath.string() + "\"";
    
    for (const auto& file : selected_file) {
        command += " \"" + file.string() + "\"";
    }
    std::cout << "Выполняется команда: " << command << std::endl;
    int result = std::system(command.c_str());

    if (result == 0) {
        std::cout << "Архив из выбранных файлов успешно создан!" << std::endl;
    }
    else {
        std::cerr << "Ошибка создания архива. Код возврата системы: " << result << std::endl;
    }
}


void EnvuploadDiscord(const std::string WebHook, const fs::path targetFile) {
    std::error_code local_ec;

    if (!fs::exists(targetFile, local_ec)) {
        std::cerr << "Ошибка: " << local_ec.value() << "\n"; 
        if(local_ec) {
            std::cerr << "Ошибка: " << local_ec.value() << " Описание: " << local_ec.message() << endl; 
        }
        return; 
        
    }
    SetEnvironmentVariableA("DISCORD_URL", WebHook.c_str());
    std::string command = "powershell -Command \"[Net.ServicePointManager]::SecurityProtocol = 'Tls12'; (New-Object System.Net.WebClient).UploadFile($env:DISCORD_URL, '" + targetFile.string() + "')\"";
    std::system(command.c_str());
}

void uploadDiscord(Fl_Widget* widget, void* data) {
    
    std::string webhookurl = webhook_input->value();
    if (webhookurl.empty()) {
        std::cerr << "Ошибка: URL вебхука пуст " << endl;
        return;
    }
    fs::path uploadZip = programmDir / "archive.zip";
    EnvuploadDiscord(webhookurl, uploadZip);

}
//Основа 
int main(int argc, char** argv) {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    std::locale::global(std::locale(".65001"));
    std::cout.imbue(std::locale());
    std::cerr.imbue(std::locale());

    Fl_Window window(480, 240, "App");

    //Кнопка для выбора файла 
    Fl_Button file_button(100, 0, 125, 30, "Выбрать файлы");
    file_button.callback(fileSoc);
    //Кнопка для копирования файла 
    Fl_Button coped_file(0, 30, 100, 30, "Копирование");
    coped_file.callback(button_copoedFiles);
    //Кнопка для выбора папки
    Fl_Button dir_directory(0, 0, 100, 30, "Выбор к папке");
    dir_directory.callback(dirSoc);
    //Кнопка для создания zip
    Fl_Button button_createZip(100, 30, 100, 30, "Сделать zip");
    button_createZip.callback(button_ZipArhive);
    // Графическое поле для ввода Webhook 
    webhook_input = new Fl_Input(10, 120, 380, 30, "Discord webHook URL:");
    webhook_input->align(FL_ALIGN_TOP_LEFT);

    // Кнопка отправки
    Fl_Button button_uploadDiscord(10, 170, 380, 40, "Отправить");
    button_uploadDiscord.callback(uploadDiscord);

    //Для вывода окна 
    window.end();
    window.show();
    return Fl::run();
}

