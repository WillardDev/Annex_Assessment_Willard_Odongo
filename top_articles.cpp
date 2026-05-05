#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <curl/curl.h>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* out) {
    size_t total = size * nmemb;
    out->append(static_cast<char*>(contents), total);
    return total;
}

std::string fetchUrl(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) return "";

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) return "";
    return response;
}

struct Article {
    std::string name;
    int comments;
};

std::vector<std::string> topArticles(int limit) {
    const std::string base = "https://jsonmock.hackerrank.com/api/articles?page=";

    std::string body = fetchUrl(base + "1");
    if (body.empty()) return {};

    json root = json::parse(body, nullptr, false);
    if (root.is_discarded()) return {};

    int totalPages = root.value("total_pages", 1);

    std::vector<Article> articles;

    auto processPage = [&](const json& pageRoot) {
        if (!pageRoot.contains("data") || !pageRoot["data"].is_array()) return;

        for (const auto& item : pageRoot["data"]) {
            std::string name;

            if (item.contains("title") && !item["title"].is_null()) {
                name = item["title"].get<std::string>();
            } else if (item.contains("story_title") && !item["story_title"].is_null()) {
                name = item["story_title"].get<std::string>();
            } else {
                continue;
            }

            int comments = 0;
            if (item.contains("num_comments") && !item["num_comments"].is_null()) {
                comments = item["num_comments"].get<int>();
            }

            articles.push_back({name, comments});
        }
    };

    processPage(root);

    for (int page = 2; page <= totalPages; page++) {
        std::string pageBody = fetchUrl(base + std::to_string(page));
        if (pageBody.empty()) continue;

        json pageRoot = json::parse(pageBody, nullptr, false);
        if (!pageRoot.is_discarded()) {
            processPage(pageRoot);
        }
    }

    std::sort(articles.begin(), articles.end(), [](const Article& a, const Article& b) {
        if (a.comments != b.comments) return a.comments > b.comments;
        return a.name > b.name;
    });

    std::vector<std::string> result;
    int take = std::min(limit, static_cast<int>(articles.size()));
    for (int i = 0; i < take; i++) {
        result.push_back(articles[i].name);
    }

    return result;
}

int main() {
    int limit;
    std::cout << "Enter limit: ";
    std::cin >> limit;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    std::vector<std::string> top = topArticles(limit);
    for (const auto& title : top) {
        std::cout << title << "\n";
    }

    curl_global_cleanup();
    return 0;
}
