#include "Factory.h"

Factory::Factory(){
}

Factory::~Factory(){
}

void Factory::startProduction(int num_products, Product* products,unsigned int id){
}

void Factory::produce(int num_products, Product* products){
}

void Factory::finishProduction(unsigned int id){
}

void Factory::startSimpleBuyer(unsigned int id){
}

int Factory::tryBuyOne(){
    return -1;
}

int Factory::finishSimpleBuyer(unsigned int id){
    return -1;
}

void Factory::startCompanyBuyer(int num_products, int min_value,unsigned int id){
}

std::list<Product> Factory::buyProducts(int num_products){
    return std::list<Product>();
}

void Factory::returnProducts(std::list<Product> products,unsigned int id){
}

int Factory::finishCompanyBuyer(unsigned int id){
    return 0;
}

void Factory::startThief(int num_products,unsigned int fake_id){
    ThiefsArrived++;
    pthread_t thiefThread;
    mapID.insert(std::pair<int,pthread_t>(fake_id,thiefThread));
    inputForComp arg;
    arg->num_products= num_products;
    arg->ID = fake_id;
    pthread_create(&thiefThread,NULL, stealProducts,(void*)arg );
    mapID.erase(fake_id);
    ThiefsArrived--;
}

int Factory::stealProducts(int num_products,unsigned int fake_id){

    //TODO put the products stole in the list of stolen products
    pthread_mutex_lock(&m);
    while(!openForVisitors){
        pthread_cond_wait(&FactoryIsOpen,&m);
    }

    for (int i = 0; i <num_products && i< availableProducts.size(); ++i) {
        stolenProductsList.push_back<fake_id,availableProducts.front()>;
        availableProducts.pop_front();
    }
    pthread_mutex_unlock(&m);
    return i;
}

int Factory::finishThief(unsigned int fake_id){
    //TODO return the num that we stole and exit?
    return 0;
}

void Factory::closeFactory(){
}

void Factory::openFactory(){
}

void Factory::closeReturningService(){
}

void Factory::openReturningService(){
}

std::list<std::pair<Product, int>> Factory::listStolenProducts(){
    //TODO return the Thief's list of all of the products they stole
    return std::list<std::pair<Product,int>>();
}

std::list<Product> Factory::listAvailableProducts(){
    return std::list<Product>();
}
static

