#ifndef FACTORY_H_
#define FACTORY_H_

#include <pthread.h>
#include <list>
#include <map>
#include "Product.h"
#include <math.h>
class Factory;

struct in_prod{
    int num_products;
    Product* products;
    Factory* f;
};
typedef struct in_prod* inputForProduce;
struct in_comp{
    int num_products;
    int ID;
    int min_value;
    int* returned_num;
    Factory* f;
};
typedef struct in_comp* inputForComp;
struct in_thief{
    int num_products;
    int fake_ID;
    int* returned_num;
    Factory* f;
};
typedef struct in_thief* inputForThief;
class Factory{
    bool openForVisitors;
    bool openForReturns;
    std::map<int,pthread_t> mapID;
    bool someoneInside;
    pthread_mutex_t m;
    std::list<Product> availableProducts;
    int ThiefsArrived;
    int companyArrived;
    std::list<std::pair<Product,int>> stolenProducts;
    pthread_cond_t priority;
    pthread_cond_t FactoryIsOpen;
    pthread_cond_t FactoryIsOpenForReturns;

public:
    Factory();
    ~Factory();

    void startProduction(int num_products, Product* products, unsigned int id);

    //void* produceAux(void* produce_param);

    void produce(int num_products, Product* products);
    void finishProduction(unsigned int id);

    void startSimpleBuyer(unsigned int id);

    void* tryBuyOneAux(void* param);

    int tryBuyOne();

    int finishSimpleBuyer(unsigned int id);

    void startCompanyBuyer(int num_products, int min_value,unsigned int id);

    void* companyFuncAux(void* arg);

    std::list<Product> buyProducts(int num_products);

    void returnProducts(std::list<Product> products,unsigned int id);

    int finishCompanyBuyer(unsigned int id);


    void startThief(int num_products,unsigned int fake_id);

    void* ThiefFuncAux(void* arg);

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
