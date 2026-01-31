
// void run_janitor(DbSyncEngine &engine)
// {
//     while (true)
//     {
//         for (int i = 0; i < engine.get_shard_count(); ++i)
//         {
//             auto &s = engine.get_shard(i);

//             // Try to lock without blocking the 4M RPS reactors
//             std::unique_lock lock(s.mtx, std::try_to_lock);
//             if (!lock.owns_lock() || s.data.empty())
//                 continue;

//             int expired_found = 0;
//             int sampled = 0;
//             auto it = s.data.begin();

//             // Random sampling: check up to 20 keys
//             while (sampled < 20 && it != s.data.end())
//             {
//                 if (it->second.expires_at > 0 && it->second.expires_at < engine.get_now())
//                 {
//                     // Capture current, then increment it BEFORE erasing
//                     auto current = it++;
//                     s.data.erase(current);
//                     expired_found++;
//                 }
//                 else
//                 {
//                     ++it;
//                 }
//                 sampled++;
//             }

//             // If the shard is "dirty" (>25% expired), we don't sleep
//             if (expired_found > 1)
//                 continue;
//         }
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     }
// }
