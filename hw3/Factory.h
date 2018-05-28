#ifndef FACTORY_H_
#define FACTORY_H_

#include <pthread.h>
#include <list>
#include <map>
#include "Product.h"
struct in_prod{
    int num_products;
    Product* products;
};
typedef struct in_prod* inputForProduce;

class Factory{
    bool openForVisitors;
    bool openForReturns;
    std::map<int,pthread_t> mapID;
    bool someoneInside;
    pthread_mutex_t m;
    std::list<Product> availableProducts;
    int ThiefsArrived;
    int companyArrived;
    pthread_cond_t priority;

public:
    Factory(): openForVisitors(true),openForReturns(true),someoneInside(false),
    ThiefsArrived(0),companyArrived(0)
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init (&attr);
        pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
        pthread_mutex_init (&priority, &attr);
    };
    ~Factory();
    
    void startProduction(int num_products, Product* products, unsigned int id)
    {
        pthread_t productionThread;
        mapID.insert(std::pair<int,pthread_t>(id,productionThread));
        inputForProduce args_to_prod;
        args_to_prod->num_products=num_products;
        args_to_prod->products=products;
        pthread_create(&productionThread,NULL,produceAux,(void*)args_to_prod);
        mapID.erase(id);
    }
    void* produceAux(void* produce_param)
    {
        produce(((inputForProduce)produce_param)->num_products,((inputForProduce)produce_param)->products);
    }
    void produce(int num_products, Product* products)
    {
        pthread_mutex_lock(&m);
        for(int i=0;i<num_products;i++)
        {
            availableProducts.push_back(products[i]);
        }
        pthread_mutex_unlock(&m);
    }
    void finishProduction(unsigned int id)
    {
        pthread_t finished_prod;
        finished_prod=mapID[id];
        pthread_join(finished_prod,NULL);
    }
    
    void startSimpleBuyer(unsigned int id)
    {
        pthread_t SimpleBuyerThread;
        mapID.insert(std::pair<int,pthread_t>(id,SimpleBuyerThread));
        pthread_create(&SimpleBuyerThread,NULL,tryBuyOneAux,NULL);
        mapID.erase(id);
    }
    void* tryBuyOneAux(void* param)
    {
        *((int*)param)=tryBuyOne();
        return param;
    }
    int tryBuyOne()
    {
        if (ThiefsArrived>0 || companyArrived>0 || pthread_mutex_trylock(&m)!=0)
        {
            return -1;
        }
        int boughtProductID=0;
        if (availableProducts.size()>0)
        {
            boughtProductID = availableProducts.begin()->getId();
            availableProducts.pop_front();
            pthread_mutex_unlock(&m);
            return boughtProductID;
        }
        else
        {
            pthread_mutex_unlock(&m);
            return -1;
        }

    }
    int finishSimpleBuyer(unsigned int id)
    {
        pthread_t finished_prod;
        finished_prod=mapID[id];
        int res=0;
        int* res_add=&res;
        int** res_add_of_add=&res_add;
        pthread_join(finished_prod,(void**)res_add_of_add);
        return **res_add_of_add;
    }

    void startCompanyBuyer(int num_products, int min_value,unsigned int id);
    std::list<Product> buyProducts(int num_products);
    void returnProducts(std::list<Product> products,unsigned int id);
    int finishCompanyBuyer(unsigned int id);

    void startThief(int num_products,unsigned int fake_id);
    int stealProducts(int num_products,unsigned int fake_id);
    int finishThief(unsigned int fake_id);

    void closeFactory();
    void openFactory();
    
    void closeReturningService();
    void openReturningService();
    
    std::list<std::pair<Product, int>> listStolenProducts();
    std::list<Product> listAvailableProducts();

};
#endif // FACTORY_H_
