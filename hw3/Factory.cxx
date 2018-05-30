#include "Factory.h"


Factory::Factory() : openForVisitors(true),openForReturns(true),
          ThiefsArrived(0),companyArrived(0)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init (&attr);
    pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
    pthread_mutex_init (&m, &attr);
    pthread_cond_init(&priority,NULL);
    pthread_cond_init(&FactoryIsOpen,NULL);
    pthread_cond_init(&FactoryIsOpenForReturns,NULL);

};

Factory::~Factory(){
}
void Factory::produce(int num_products, Product* products)
{
    pthread_mutex_lock(&m);
    for(int i=0;i<num_products;i++)
    {
        availableProducts.push_back(products[i]);
    }
    pthread_mutex_unlock(&m);
}

void* produceAux(void* produce_param)
{
    ((inputForProduce)(produce_param))->f->produce(((inputForProduce)produce_param)->num_products,((inputForProduce)produce_param)->products);
}

void Factory::startProduction(int num_products, Product* products,unsigned int id)
{
    pthread_t productionThread;
    mapID.insert(std::pair<int,pthread_t>(id,productionThread));
    inputForProduce args_to_prod;
    args_to_prod->num_products=num_products;
    args_to_prod->products=products;
    args_to_prod->f=this;
    pthread_create(&productionThread,NULL,&produceAux,(void*)args_to_prod);
    mapID.erase(id);
}




void Factory::finishProduction(unsigned int id)
{
    pthread_t finished_prod=mapID[id];
    pthread_join(finished_prod,NULL);
}
void* tryBuyOneAux(void* param)
{
    inputForSimpleBuyer* actual_param = (inputForSimpleBuyer*) param;
    int* result = new int();
    *result = actual_param->f->tryBuyOne();
    //printf("got here2");
    //int a=((inputForSimpleBuyer*)(param))->f->tryBuyOne();
    //printf("this is tryBuyOne aux with %d\n",a);
    actual_param->id_of_bought_prod=actual_param->f->tryBuyOne();
    //((inputForSimpleBuyer*)(param))->id_of_bought_prod=8;
    //printf("got here3");
    //printf("aux:%d\n",((inputForSimpleBuyer*)(param))->id_of_bought_prod);
    return result;
}

void Factory::startSimpleBuyer(unsigned int id)
{
    pthread_t SimpleBuyerThread;
    //mapID.insert(std::pair<int,pthread_t&>(id,SimpleBuyerThread));
    mapID[id] = SimpleBuyerThread;
    inputForSimpleBuyer* args= new inputForSimpleBuyer();
    args->f=this;
    int ans=pthread_create(&mapID[id],NULL,&tryBuyOneAux,(void*)(args));
    printf("is this that %d\n",args->id_of_bought_prod);
}

int Factory::tryBuyOne()
{    /// because of piazza, not according to instructions!!!
    //if (openForVisitors== false || ThiefsArrived>0 || companyArrived>0 || pthread_mutex_trylock(&m)!=0)
    if (openForVisitors== false || pthread_mutex_trylock(&m)!= 0)
    {
        printf("try\n");
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

int Factory::finishSimpleBuyer(unsigned int id)
{
   // pthread_mutex_lock(&m);
    printf("6564");
    pthread_t finished_simpleBuyer;
    finished_simpleBuyer=mapID[id];
    int* res_add=nullptr;
    int** res_add_of_add=&res_add;
    int success = pthread_join(finished_simpleBuyer,(void**)res_add_of_add);
    printf("finish:%d\n",**res_add_of_add);
    mapID.erase(id);
    int return_val = *res_add;
    delete res_add;
    // pthread_mutex_unlock(&m);
    return return_val;
}
void* companyFuncAux(void* arg)
{
    std::list<Product> boughtByCompany=((inputForComp)(arg))->f->buyProducts(((inputForComp)arg)->num_products);
    std::list<Product> productsToReturn;
    std::list<Product>::iterator it;
    for (it = boughtByCompany.begin(); it != boughtByCompany.end(); ++it)
    {
        if (it->getValue()<((inputForComp)arg)->min_value)
        {
            productsToReturn.push_back(*it);
        }
    }
    ((inputForComp)(arg))->f->returnProducts(productsToReturn,((inputForComp)arg)->ID);
    *(((inputForComp)arg)->returned_num) =productsToReturn.size();
    return (((inputForComp)arg)->returned_num) ;
}
void Factory::startCompanyBuyer(int num_products, int min_value,unsigned int id)
{
    companyArrived++;
    pthread_t CompanyThread;
    mapID.insert(std::pair<int,pthread_t&>(id,CompanyThread));
    inputForComp arg;
    arg->num_products=num_products;
    arg->ID=id;
    arg->min_value=min_value;
    pthread_create(&CompanyThread,NULL,companyFuncAux,(void*)(arg));
    mapID.erase(id);
    companyArrived--;
}


std::list<Product> Factory::buyProducts(int num_products)
{
    pthread_mutex_lock(&m);
    while (openForVisitors== false)
    {
        pthread_cond_wait(&FactoryIsOpen,&m);
    }
    while (ThiefsArrived>0 || availableProducts.size()<num_products)
    {
        pthread_cond_wait(&priority,&m);
    }
    std::list<Product> boughtProd;
    for (int i=0;i<num_products;i++)
    {
        boughtProd.push_back(*availableProducts.begin());
        availableProducts.pop_front();
    }
    pthread_mutex_unlock(&m);
}

void Factory::returnProducts(std::list<Product> products,unsigned int id)
{
    pthread_mutex_lock(&m);
    while (openForVisitors== false)
    {
        pthread_cond_wait(&FactoryIsOpenForReturns,&m);
    }
    while (ThiefsArrived>0)
    {
        pthread_cond_wait(&priority,&m);
    }
    for (int i=0;i<products.size();i++)
    {
        availableProducts.push_back(*products.begin());
        products.pop_front();
    }
    pthread_mutex_unlock(&m);
}

int Factory::finishCompanyBuyer(unsigned int id)
{
    pthread_t finished_company;
    finished_company=mapID[id];
    int res=0;
    int* res_add=&res;
    int** res_add_of_add=&res_add;
    pthread_join(finished_company,(void**)res_add_of_add);
    return **res_add_of_add;
}
void* ThiefFuncAux(void* arg)
{
    int stolenNum= ((inputForThief)(arg))->f->stealProducts(((inputForThief)(arg))->num_products,((inputForThief)(arg))->fake_ID);
    *(((inputForThief)(arg))->returned_num)=stolenNum;
    return ((inputForThief)(arg))->returned_num;
}
void Factory::startThief(int num_products,unsigned int fake_id)
{
    ThiefsArrived++;
    pthread_t ThiefThread;
    mapID.insert(std::pair<int,pthread_t&>(fake_id,ThiefThread));
    inputForThief arg;
    arg->num_products=num_products;
    arg->fake_ID=fake_id;
    pthread_create(&ThiefThread,NULL,ThiefFuncAux,(void*)(arg));
    mapID.erase(fake_id);
    ThiefsArrived--;
    if (ThiefsArrived==0)
    {
        pthread_mutex_lock(&m);
        pthread_cond_broadcast(&priority);
        pthread_mutex_unlock(&m);
    }
}


int Factory::stealProducts(int num_products,unsigned int fake_id)
{
    pthread_mutex_lock(&m);
    while (openForVisitors== false)
    {
        pthread_cond_wait(&FactoryIsOpen,&m);
    }
    int possibleToSteal=0;
    if(num_products<availableProducts.size())
    {
        possibleToSteal=num_products;
    }
    else
    {
        possibleToSteal=availableProducts.size();
    }
    for (int j=0;j<possibleToSteal;j++)
    {
        stolenProducts.push_back(std::pair<Product,int>(*availableProducts.begin(),fake_id));
        availableProducts.pop_front();
    }
    pthread_mutex_unlock(&m);
    return  possibleToSteal;
}

int Factory::finishThief(unsigned int fake_id)
{
    pthread_t finished_thief;
    finished_thief=mapID[fake_id];
    int res=0;
    int* res_add=&res;
    int** res_add_of_add=&res_add;
    pthread_join(finished_thief,(void**)res_add_of_add);
    return **res_add_of_add;
}

void Factory::closeFactory()
{
    openForVisitors= false;
}

void Factory::openFactory()
{
    pthread_mutex_lock(&m);
    openForVisitors=true;
    pthread_cond_broadcast(&FactoryIsOpen);
    pthread_mutex_unlock(&m);
}

void Factory::closeReturningService()
{
    openForReturns= false;
}

void Factory::openReturningService()
{
    pthread_mutex_lock(&m);
    openForReturns=true;
    pthread_cond_broadcast(&FactoryIsOpenForReturns);
    pthread_mutex_unlock(&m);
}

std::list<std::pair<Product, int>> Factory::listStolenProducts()
{
    return stolenProducts;
}

std::list<Product> Factory::listAvailableProducts()
{
    return availableProducts;
}

