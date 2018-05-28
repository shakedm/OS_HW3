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
struct in_comp{
    int num_products;
    int ID;
};
typedef struct in_comp* inputForComp;

class Factory{
    bool openForVisitors;
    bool openForReturns;
    std::map<int,pthread_t> mapID;
    bool someoneInside;
    pthread_mutex_t m;
    std::list<Product> availableProducts;
    std::list<Pair<int,Product>> stolenProductsList;
    int ThiefsArrived;
    int companyArrived;
    pthread_cond_t priority;
    pthread_cond_t FactoryIsOpen;

public:
    Factory(): openForVisitors(true),openForReturns(true),someoneInside(false),
    ThiefsArrived(0),companyArrived(0)
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init (&attr);
        pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
        pthread_mutex_init (&priority, &attr);
        pthread_cond_init(&FactoryIsOpen, NULL);
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
        pthread_t finished_simpleBuyer;
        finished_simpleBuyer=mapID[id];
        int res=0;
        int* res_add=&res;
        int** res_add_of_add=&res_add;
        pthread_join(finished_simpleBuyer,(void**)res_add_of_add);
        return **res_add_of_add;
    }

    void startCompanyBuyer(int num_products, int min_value,unsigned int id)
    {
        companyArrived++;
        pthread_t CompanyThread;
        mapID.insert(std::pair<int,pthread_t>(id,CompanyThread));
        inputForComp arg;
        arg->num_products=num_products;
        arg->ID=id;
        pthread_create(&CompanyThread,NULL,companyFuncAux,(void*)(arg));
        mapID.erase(id);
        companyArrived--;
    }
    void* companyFuncAux(void* arg)
    {
        std::list<Product> boughtByCompany=buyProducts(((inputForComp)arg)->num_products);
        returnProducts(boughtByCompany,((inputForComp)arg)->ID);
    }
    std::list<Product> buyProducts(int num_products)
    {
        pthread_mutex_lock(&m);
        while (ThiefsArrived>0 || availableProducts.size()<num_products)
        {
            pthread_cond_wait(&priority,&m);
        }
        std::list<Product>
        for (int i=0;i<num_products;i++)
        {

        }
    }
    void returnProducts(std::list<Product> products,unsigned int id);
    int finishCompanyBuyer(unsigned int id);

    void startThief(int num_products,unsigned int fake_id);
    int stealProducts(int num_products,unsigned int fake_id);
    int finishThief(unsigned int fake_id);

    void closeFactory(){
        openForVisitors = FALSE;
    }
    void openFactory(){
        openForVisitors = TRUE;
    }
    
    void closeReturningService(){
        openForReturns = FALSE;
    }
    void openReturningService(){
        openForReturns = TRUE;
    }
    
    std::list<std::pair<Product, int>> listStolenProducts();
    std::list<Product> listAvailableProducts();

};
#endif // FACTORY_H_
