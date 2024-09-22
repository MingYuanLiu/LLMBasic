#include <iostream>
#include <curl/curl.h>

int main() {
    CURL *curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://curl.se/docs/caextract.html");

#ifdef CA_Certificate_Path
        curl_easy_setopt(curl, CURLOPT_CAINFO, CA_Certificate_Path);
#endif
        
        CURLcode res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
    }
    return 0;
}
