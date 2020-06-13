#pragma once

#include <algorithm>
#include <shared_mutex>
#include <unordered_map>
#include <utility>

#include "core/definitions.h"

namespace thorwe
{

using namespace com::teamspeak;

template <typename T> class Client_Storage
{

  public:
    std::pair<T *, bool>
    add_item(connection_id_t connection_id, client_id_t client_id, std::unique_ptr<T> new_item)
    {
        if (get(connection_id, client_id))
            return {nullptr, false};

        auto &&connections = m_storage[connection_id];
        auto result = connections.emplace(client_id, std::move(new_item));
        return {result.first->second.get(), result.second};
    }

    bool delete_item(connection_id_t connection_id, client_id_t client_id)
    {

        if (auto connection_it = m_storage.find(connection_id); connection_it != std::end(m_storage))
        {
            auto &&client_map = connection_it->second;
            if (auto client_it = client_map.find(client_id); client_it != std::end(client_map))
            {
                client_map.erase(client_it);

                if (client_map.size() == 0)
                    m_storage.erase(connection_it);

                return true;
            }
        }
        return false;
    }

    void delete_items(connection_id_t connection_id = 0)
    {
        if (connection_id == 0)
            m_storage.clear();
        else
        {
            auto connection_it = m_storage.find(connection_id);
            if (connection_it != std::end(m_storage))
                m_storage.erase(connection_it);
        }
    }

    // internal use
    T *get(connection_id_t connection_id, client_id_t client_id)
    {
        const auto &storage = m_storage;
        auto connection_it = m_storage.find(connection_id);
        if (connection_it == std::cend(m_storage))
            return nullptr;

        const auto &client_map = connection_it->second;
        auto client_it = client_map.find(client_id);
        if (client_it == std::cend(client_map))
            return nullptr;

        return client_it->second.get();
    }

    template <typename U> auto do_for(U &&fn, connection_id_t connection_id, client_id_t client_id)
    {
        const auto &storage = m_storage;
        if (auto connection_it = storage.find(connection_id); connection_it != std::cend(storage))
        {
            const auto &clients_storage = connection_it->second;
            if (auto clients_storage_it = clients_storage.find(client_id);
                clients_storage_it != std::cend(clients_storage))
                return fn(clients_storage_it->second.get());
        }
        return fn(nullptr);
    }

    template <typename U> void do_for_each(U &&fn, connection_id_t connection_id = 0)
    {
        if (connection_id == 0)
        {
            for (const auto &connection : m_storage)
            {
                for (const auto &client : connection.second)
                    fn(client.second.get());
            }
        }
        else
        {
            const auto &storage = m_storage;
            if (auto connection_it = storage.find(connection_id); connection_it != std::cend(storage))
            {
                for (const auto &client : connection_it->second)
                    fn(client.second.get());
            }
        }
    }

  private:
    using ConnectionMap = std::unordered_map<client_id_t, std::unique_ptr<T>>;
    std::unordered_map<connection_id_t, ConnectionMap> m_storage;
};

template <typename T> class Safe_Client_Storage
{

  private:
    std::shared_mutex m_mutex;

  public:
    std::pair<T *, bool>
    add_item(connection_id_t connection_id, client_id_t client_id, std::unique_ptr<T> item)
    {
        std::unique_lock<decltype(m_mutex)> lock(m_mutex);
        return m_storage.add_item(connection_id, client_id, std::move(item));
    }
    bool delete_item(connection_id_t connection_id, client_id_t client_id)
    {
        std::unique_lock<decltype(m_mutex)> lock(m_mutex);
        return m_storage.delete_item(connection_id, client_id);
    }
    void delete_items(connection_id_t connection_id = 0)
    {
        std::unique_lock<decltype(m_mutex)> lock(m_mutex);
        m_storage.delete_items(connection_id);
    }
    bool contains(connection_id_t connection_id, client_id_t client_id)
    {
        std::shared_lock<decltype(m_mutex)> lock(m_mutex);
        return m_storage.get(connection_id, client_id) != nullptr;
    }

    template <typename U> auto do_for(U &&fn, connection_id_t connection_id, client_id_t client_id)
    {
        std::shared_lock<decltype(m_mutex)> lock(m_mutex);
        return m_storage.do_for(fn, connection_id, client_id);
    }

    template <typename U> void do_for_each(U &&fn, connection_id_t connection_id = 0)
    {
        std::shared_lock<decltype(m_mutex)> lock(m_mutex);
        m_storage.do_for_each(fn, connection_id);
    }

  private:
    Client_Storage<T> m_storage;
};

}  // namespace thorwe
