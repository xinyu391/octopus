

```C++
int func(int i) {
    ...
    return 0;
}
// 3个线程，6个任务队列，如果参数二为-1，时队列“无限”大
    octopus::Pool pool(3);
    std::vector<std::future<int>> rets;
    for (int i = 0; i < 15; i++) {
         
        // 添加任务到队列，当任务队列满时，add函数会阻塞
        auto tid = pool.add(func, 4);
        rets.push_back(std::move(tid));
    }
    
    //等待任务结束
    for(int i=0;i<rets.size();i++){
        rets[i].get();
    }

```