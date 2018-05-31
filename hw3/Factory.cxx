#include "Factory.h"


Factory::Factory() : openForVisitors(true),openForReturns(true),
          ThiefsArrived(0),companyArrived(0)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init (&attr);
    pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
    pthread_mutex_init (&m, &attr);
    pthread_cond_init(&priority,NULL);
    pthread_cond_init(&waitProd, NULL);
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
    pthread_cond_broadcast(&waitProd);
    pthread_mutex_unlock(&m);
}

void* produceAux(void* produce_param)
{
    ((inputForProduce)(produce_param))->f->produce(((inputForProduce)produce_param)->num_products,((inputForProduce)produce_param)->products);
}

void Factory::startProduction(int num_products, Product* products,unsigned int id)
{
    pthread_t productionThread;

    inputForProduce args_to_prod= new in_prod();
    args_to_prod->num_products=num_products;
    args_to_prod->products=products;
    args_to_prod->f=this;
    pthread_create(&productionThread,NULL,&produceAux,(void*)args_to_prod);
    mapID[id]= productionThread;

}




void Factory::finishProduction(unsigned int id)
{
    pthread_t finished_prod=mapID[id];
    ///whaaatttttt???? delete? return?
    pthread_join(finished_prod,NULL);
}
void* tryBuyOneAux(void* param)
{
    inputForSimpleBuyer* actual_param = (inputForSimpleBuyer*) param;
    int* result = new int();
    *result = actual_param->f->tryBuyOne();
    delete  actual_param;

    return result;
}

void Factory::startSimpleBuyer(unsigned int id)
{
    pthread_t SimpleBuyerThread;

    inputForSimpleBuyer* args = new inputForSimpleBuyer;
    args->f=this;

    int ans=pthread_create(&SimpleBuyerThread,NULL,&tryBuyOneAux,(void*)(args));
    mapID[id] = SimpleBuyerThread;

}

int Factory::tryBuyOne()
{    /// because of piazza, not according to instructions!!!

    if (openForVisitors== false || pthread_mutex_trylock(&m)!= 0)
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

int Factory::finishSimpleBuyer(unsigned int id)
{
    pthread_t finished_simpleBuyer;
    finished_simpleBuyer=mapID[id];
    int* res_add=nullptr;
    int** res_add_of_add=&res_add;
    int success = pthread_join(finished_simpleBuyer,(void**)res_add_of_add);
    mapID.erase(id);
    int return_val = *res_add;
    delete res_add;
    return return_val;
}
void* companyFuncAux(void* arg)
{
    std::list<Product> boughtByCompany=((inputForComp)(arg))->f->buyProducts(((inputForComp)arg)->num_products);

    std::list<Product> productsToReturn;
    std::list<Product>::iterator it;
    //we need to return only if the
    for (it = boughtByCompany.begin(); it != boughtByCompany.end(); ++it)
    {
        if (it->getValue()<((inputForComp)arg)->min_value)
        {
            productsToReturn.push_back(*it);
        }
    }
    int *result = new int (productsToReturn.size());
    if(productsToReturn.size() == 0){
        delete (inputForComp)arg;
        return result;
    }
    ((inputForComp)(arg))->f->returnProducts(productsToReturn,((inputForComp)arg)->ID);
    *result = productsToReturn.size();

    delete (inputForComp)(arg);
    return result;
}
void Factory::startCompanyBuyer(int num_products, int min_value,unsigned int id)
{
    //should be locked somewhere when locked do: companyArrived--;
    pthread_t CompanyThread;

    inputForComp arg= new in_comp();
    arg->num_products=num_products;
    arg->ID=id;
    arg->min_value=min_value;
    arg->f=this;
    pthread_create(&CompanyThread,NULL,companyFuncAux,(void*)(arg));
    mapID[id]= CompanyThread;
    //companyArrived--;
}


std::list<Product> Factory::buyProducts(int num_products)
{
    //TODO do we need to lock here?
    companyArrived++;
    pthread_mutex_lock(&m);
    while (openForVisitors== false)
    {
        pthread_cond_wait(&FactoryIsOpen,&m);
    }
    while (ThiefsArrived>0 )
    {
        pthread_cond_wait(&priority,&m);
    }
    while(availableProducts.size()<num_products){
        companyArrived--;
        pthread_cond_wait(&waitProd,&m);
        companyArrived++;
    }
    std::list<Product> boughtProd;
    for (int i=0;i<num_products;i++)
    {
        boughtProd.push_back(*availableProducts.begin());
        availableProducts.pop_front();
    }
    companyArrived--;
    pthread_mutex_unlock(&m);
    return  boughtProd;
}

void Factory::returnProducts(std::list<Product> products,unsigned int id)
{
    pthread_mutex_lock(&m);
    while(openForVisitors == false){
        pthread_cond_wait(&FactoryIsOpen,&m);
    }
    while (openForReturns== false)
    {
        pthread_cond_wait(&FactoryIsOpenForReturns,&m);
    }
    while (ThiefsArrived>0)
    {
        pthread_cond_wait(&priority,&m);
    }
    int size = products.size();
    for (int i = 0;i < size ;i++)
    {
        availableProducts.push_back(*products.begin());
        products.pop_front();
    }
    pthread_cond_broadcast(&waitProd);
    pthread_mutex_unlock(&m);
}

int Factory::finishCompanyBuyer(unsigned int id)
{
    pthread_t finished_company;
    finished_company=mapID[id];
    int* res_add= nullptr;
    int** res_add_of_add=&res_add;
    int what = pthread_join(finished_company,(void**)res_add_of_add);
    // why do we need "what" value?
    mapID.erase(id);
    int ret_val = *res_add;
    delete res_add;//what? we need to delete the input for company?
    return ret_val;
}
void* ThiefFuncAux(void* arg)
{
    int stolenNum= ((inputForThief)(arg))->f->stealProducts(((inputForThief)(arg))->num_products,((inputForThief)(arg))->fake_ID);
    int* res = new int(stolenNum);
    delete (inputForThief)(arg);
    return res;
}
void Factory::startThief(int num_products,unsigned int fake_id)
{
    pthread_t ThiefThread;

    inputForThief arg = new in_thief();
    arg->num_products = num_products;
    arg->fake_ID = fake_id;
    arg->f= this;
    ThiefsArrived++;
    pthread_create(&ThiefThread,NULL,ThiefFuncAux,(void*)(arg));
    mapID[fake_id]= ThiefThread;

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
    ThiefsArrived--;
    if (ThiefsArrived==0)
    {
        pthread_cond_broadcast(&priority);
    }
    pthread_mutex_unlock(&m);
    return  possibleToSteal;
}

int Factory::finishThief(unsigned int fake_id)
{
    pthread_t finished_thief;
    finished_thief=mapID[fake_id];
    int* res_add= nullptr;
    int** res_add_of_add=&res_add;
    pthread_join(finished_thief,(void**)res_add_of_add);
    mapID.erase(fake_id);
    int return_val = *res_add;
    delete res_add;
    return return_val;
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

