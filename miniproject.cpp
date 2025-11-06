#include <iostream>
#include <string>
#include <curl/curl.h>
#include <cstdlib> // for getenv

// Callback to collect API response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t total_size = size * nmemb;
    response->append((char*)contents, total_size);
    std::cout << "📥 Received " << total_size << " bytes in callback\n";
    return total_size;
}

int main() {
    std::cout << "🚀 Program started...\n";

    // Get API key from environment variable
    const char* key_env = getenv("GEMINI_API_KEY");
    if (!key_env) {
        std::cerr << "❌ Error: GEMINI_API_KEY not set.\n";
        std::cout << "💡 Please set the environment variable first:\n";
        std::cout << "   In PowerShell: $env:GEMINI_API_KEY='your_api_key_here'\n";
        std::cout << "   In Command Prompt: set GEMINI_API_KEY=your_api_key_here\n";
        std::cout << "Press Enter to exit...";
        std::cin.get();
        return 1;
    }
    std::string api_key = key_env;
    std::cout << "✅ API key loaded: " << api_key.substr(0, 5) << "*****\n";

    // Initialize curl
    std::cout << "🔄 Initializing CURL...\n";
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        std::cout << "✅ CURL initialized successfully\n";
        
        std::string prompt;
        std::cout << "Enter your prompt: ";
        std::getline(std::cin, prompt);

        if(prompt.empty()) {
            std::cerr << "⚠️ Prompt is empty. Exiting.\n";
            std::cout << "Press Enter to exit...";
            std::cin.get();
            return 1;
        }
        std::cout << "📝 Prompt received: " << prompt << "\n";

        // Construct API URL
        std::string url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=" + api_key;
        std::cout << "🌐 Sending request to Gemini API...\n";

        // JSON body
        std::string json_payload = R"({
            "contents": [
                {
                    "parts": [
                        {
                            "text": ")" + prompt + R"("
                        }
                    ]
                }
            ]
        })";

        std::cout << "📦 JSON payload created\n";

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::cout << "📋 Headers set\n";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L); // 30 second timeout
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); // Enable verbose output

        std::cout << "📤 Making API request...\n";
        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "❌ CURL error: " << curl_easy_strerror(res) << std::endl;
        } else {
            std::cout << "✅ API request completed. Response size: " << response.size() << " bytes\n";
            
            if (response.empty()) {
                std::cerr << "⚠️ Empty response received\n";
            } else {
                std::cout << "📄 Raw response preview: " << response.substr(0, 200) << "...\n";
            }

            if (response.find("\"error\"") != std::string::npos) {
                std::cerr << "⚠️ API Error detected in response\n";
                std::cout << "Full error response: " << response << std::endl;
            } else {
                // Find and extract only the "text" part from JSON
                std::string key = "\"text\": \"";
                size_t start = response.find(key);
                if (start != std::string::npos) {
                    start += key.length();
                    size_t end = response.find("\"", start);
                    std::string text = response.substr(start, end - start);

                    // Replace escaped \n with real newlines
                    size_t pos = 0;
                    while ((pos = text.find("\\n", pos)) != std::string::npos) {
                        text.replace(pos, 2, "\n");
                        pos += 1;
                    }

                    std::cout << "\n💬 Response from Gemini:\n" << text << "\n" << std::endl;
                } else {
                    std::cerr << "⚠️ Couldn't find 'text' in response\n";
                    std::cout << "🔍 Full response content: " << response << std::endl;
                }
            }
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        std::cerr << "❌ Failed to initialize CURL.\n";
    }

    curl_global_cleanup();
    std::cout << "🛑 Program finished.\n";
    
    // Add pause to see output in Windows
    std::cout << "Press Enter to exit...";
    std::cin.get();
    
    return 0;
}