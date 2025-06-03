#include <gtest/gtest.h>
#include "message_store.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using boost::property_tree::ptree;

// Test fixture to manage a temporary directory and clear the singleton’s state
class MessageStoreTest : public ::testing::Test {
protected:
    fs::path temp_dir;

    void SetUp() override {
        temp_dir = fs::temp_directory_path() / fs::path("message_store_test_dir");
        if (fs::exists(temp_dir)) {
            fs::remove_all(temp_dir);
        }
        fs::create_directories(temp_dir);

        // Clear the singleton
        MessageStore::instance().load_from_file(temp_dir.string());
    }

    void TearDown() override {
        // After each test, remove the directory and its contents
        if (fs::exists(temp_dir)) {
            fs::remove_all(temp_dir);
        }
        // Clear singleton
        MessageStore::instance().load_from_file(temp_dir.string());
    }
};

TEST_F(MessageStoreTest, AddAndGetAll) {
    auto initial = MessageStore::instance().get_all();
    EXPECT_TRUE(initial.empty());

    MessageStore::instance().add("Chris", "First message");
    MessageStore::instance().add("Lea",   "Second message");

    auto all = MessageStore::instance().get_all();
    ASSERT_EQ(all.size(), 2u);
    EXPECT_EQ(all[0].username,  "Chris");
    EXPECT_EQ(all[0].content,   "First message");
    EXPECT_FALSE(all[0].timestamp.empty());

    EXPECT_EQ(all[1].username,  "Lea");
    EXPECT_EQ(all[1].content,   "Second message");
    EXPECT_FALSE(all[1].timestamp.empty());
}

TEST_F(MessageStoreTest, PersistAndLoad) {
    MessageStore::instance().add("Chris", "Message 1");
    MessageStore::instance().add("Lea",   "Message 2");
    MessageStore::instance().add("Sue", "Message 3");

    auto beforePersist = MessageStore::instance().get_all();
    ASSERT_EQ(beforePersist.size(), 3u);

    MessageStore::instance().persist_to_file(temp_dir.string());
    for (size_t i = 1; i <= beforePersist.size(); ++i) {
        fs::path filename = temp_dir / (std::to_string(i) + ".json");
        EXPECT_TRUE(fs::exists(filename)) << "Expected file " << filename << " to exist.";

        ptree tree;
        std::ifstream inF(filename);
        ASSERT_TRUE(inF.is_open()) << "Could not open " << filename;
        read_json(inF, tree);
        inF.close();

        EXPECT_EQ(tree.get<std::string>("username"),  beforePersist[i - 1].username);
        EXPECT_EQ(tree.get<std::string>("content"),   beforePersist[i - 1].content);
        EXPECT_EQ(tree.get<std::string>("timestamp"), beforePersist[i - 1].timestamp);
    }
    MessageStore::instance().load_from_file(temp_dir.string());
    auto afterLoad = MessageStore::instance().get_all();

    ASSERT_EQ(afterLoad.size(), beforePersist.size());

    for (size_t i = 0; i < afterLoad.size(); ++i) {
        EXPECT_EQ(afterLoad[i].username,  beforePersist[i].username);
        EXPECT_EQ(afterLoad[i].content,   beforePersist[i].content);
        EXPECT_EQ(afterLoad[i].timestamp, beforePersist[i].timestamp);
    }
}

TEST_F(MessageStoreTest, LoadFromNonexistentDirectory) {
    fs::path bad_path = temp_dir / "no_such_dir";
    EXPECT_FALSE(fs::exists(bad_path));

    MessageStore::instance().add("Max", "Empty");
    EXPECT_FALSE(MessageStore::instance().get_all().empty());

    EXPECT_NO_THROW(MessageStore::instance().load_from_file(bad_path.string()));
    auto all = MessageStore::instance().get_all();
    EXPECT_TRUE(all.empty());
}