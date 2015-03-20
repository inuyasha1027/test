#include "my_osip_utility.hpp"

std::string to_string(int type) {
    std::string s;
    switch(type) {
    TO_STRING_ENTRY(TIMEOUT_A)
    TO_STRING_ENTRY(TIMEOUT_B)
    TO_STRING_ENTRY(TIMEOUT_D)               
    TO_STRING_ENTRY(TIMEOUT_E)             
    TO_STRING_ENTRY(TIMEOUT_F)             
    TO_STRING_ENTRY(TIMEOUT_K)             
    TO_STRING_ENTRY(TIMEOUT_G)           
    TO_STRING_ENTRY(TIMEOUT_H)                
    TO_STRING_ENTRY(TIMEOUT_I)                
    TO_STRING_ENTRY(TIMEOUT_J)                
    TO_STRING_ENTRY(RCV_REQINVITE)     
    TO_STRING_ENTRY(RCV_REQACK)            
    TO_STRING_ENTRY(RCV_REQUEST)      
    TO_STRING_ENTRY(RCV_STATUS_1XX)      
    TO_STRING_ENTRY(RCV_STATUS_2XX)      
    TO_STRING_ENTRY(RCV_STATUS_3456XX)    
    TO_STRING_ENTRY(SND_REQINVITE)      
    TO_STRING_ENTRY(SND_REQACK)            
    TO_STRING_ENTRY(SND_REQUEST)              
    TO_STRING_ENTRY(SND_STATUS_1XX)  
    TO_STRING_ENTRY(SND_STATUS_2XX)           
    TO_STRING_ENTRY(SND_STATUS_3456XX)        
    TO_STRING_ENTRY(KILL_TRANSACTION)        
    TO_STRING_ENTRY(UNKNOWN_EVT)
    default:
        assert(false);
        break;
    }
    return s;
}

std::string kill_transaction_to_string(int type) {
    std::string s;
    switch (type) {
    TO_STRING_ENTRY(OSIP_ICT_KILL_TRANSACTION)
    TO_STRING_ENTRY(OSIP_IST_KILL_TRANSACTION)
    TO_STRING_ENTRY(OSIP_NICT_KILL_TRANSACTION)
    TO_STRING_ENTRY(OSIP_NIST_KILL_TRANSACTION)
    default:
        std::cout << "wrong type:" << type << std::endl;
        break;
    }

    return s;
}

std::string message_callback_type_to_string(int type) {
    std::string s;
    switch (type) {
    TO_STRING_ENTRY(OSIP_ICT_INVITE_SENT)       
    TO_STRING_ENTRY(OSIP_ICT_INVITE_SENT_AGAIN)             
    TO_STRING_ENTRY(OSIP_ICT_ACK_SENT)                      
    TO_STRING_ENTRY(OSIP_ICT_ACK_SENT_AGAIN)                
    TO_STRING_ENTRY(OSIP_ICT_STATUS_1XX_RECEIVED)           
    TO_STRING_ENTRY(OSIP_ICT_STATUS_2XX_RECEIVED)           
    TO_STRING_ENTRY(OSIP_ICT_STATUS_2XX_RECEIVED_AGAIN)     
    TO_STRING_ENTRY(OSIP_ICT_STATUS_3XX_RECEIVED)           
    TO_STRING_ENTRY(OSIP_ICT_STATUS_4XX_RECEIVED)           
    TO_STRING_ENTRY(OSIP_ICT_STATUS_5XX_RECEIVED)           
    TO_STRING_ENTRY(OSIP_ICT_STATUS_6XX_RECEIVED)           
    TO_STRING_ENTRY(OSIP_ICT_STATUS_3456XX_RECEIVED_AGAIN)  

    TO_STRING_ENTRY(OSIP_IST_INVITE_RECEIVED)               
    TO_STRING_ENTRY(OSIP_IST_INVITE_RECEIVED_AGAIN)         
    TO_STRING_ENTRY(OSIP_IST_ACK_RECEIVED)                  
    TO_STRING_ENTRY(OSIP_IST_ACK_RECEIVED_AGAIN)            
    TO_STRING_ENTRY(OSIP_IST_STATUS_1XX_SENT)               
    TO_STRING_ENTRY(OSIP_IST_STATUS_2XX_SENT)               
    TO_STRING_ENTRY(OSIP_IST_STATUS_2XX_SENT_AGAIN)         
    TO_STRING_ENTRY(OSIP_IST_STATUS_3XX_SENT)               
    TO_STRING_ENTRY(OSIP_IST_STATUS_4XX_SENT)               
    TO_STRING_ENTRY(OSIP_IST_STATUS_5XX_SENT)               
    TO_STRING_ENTRY(OSIP_IST_STATUS_6XX_SENT)               
    TO_STRING_ENTRY(OSIP_IST_STATUS_3456XX_SENT_AGAIN)      

    TO_STRING_ENTRY(OSIP_NICT_REGISTER_SENT)                
    TO_STRING_ENTRY(OSIP_NICT_BYE_SENT)                     
    TO_STRING_ENTRY(OSIP_NICT_OPTIONS_SENT)                 
    TO_STRING_ENTRY(OSIP_NICT_INFO_SENT)                    
    TO_STRING_ENTRY(OSIP_NICT_CANCEL_SENT)                  
    TO_STRING_ENTRY(OSIP_NICT_NOTIFY_SENT)                  
    TO_STRING_ENTRY(OSIP_NICT_SUBSCRIBE_SENT)               
    TO_STRING_ENTRY(OSIP_NICT_UNKNOWN_REQUEST_SENT)         
    TO_STRING_ENTRY(OSIP_NICT_REQUEST_SENT_AGAIN)           
    TO_STRING_ENTRY(OSIP_NICT_STATUS_1XX_RECEIVED)          
    TO_STRING_ENTRY(OSIP_NICT_STATUS_2XX_RECEIVED)          
    TO_STRING_ENTRY(OSIP_NICT_STATUS_2XX_RECEIVED_AGAIN)    
    TO_STRING_ENTRY(OSIP_NICT_STATUS_3XX_RECEIVED)          
    TO_STRING_ENTRY(OSIP_NICT_STATUS_4XX_RECEIVED)          
    TO_STRING_ENTRY(OSIP_NICT_STATUS_5XX_RECEIVED)          
    TO_STRING_ENTRY(OSIP_NICT_STATUS_6XX_RECEIVED)          
    TO_STRING_ENTRY(OSIP_NICT_STATUS_3456XX_RECEIVED_AGAIN) 

    TO_STRING_ENTRY(OSIP_NIST_REGISTER_RECEIVED)            
    TO_STRING_ENTRY(OSIP_NIST_BYE_RECEIVED)                 
    TO_STRING_ENTRY(OSIP_NIST_OPTIONS_RECEIVED)             
    TO_STRING_ENTRY(OSIP_NIST_INFO_RECEIVED)                
    TO_STRING_ENTRY(OSIP_NIST_CANCEL_RECEIVED)              
    TO_STRING_ENTRY(OSIP_NIST_NOTIFY_RECEIVED)              
    TO_STRING_ENTRY(OSIP_NIST_SUBSCRIBE_RECEIVED)           

    TO_STRING_ENTRY(OSIP_NIST_UNKNOWN_REQUEST_RECEIVED)     
    TO_STRING_ENTRY(OSIP_NIST_REQUEST_RECEIVED_AGAIN)       
    TO_STRING_ENTRY(OSIP_NIST_STATUS_1XX_SENT)              
    TO_STRING_ENTRY(OSIP_NIST_STATUS_2XX_SENT)              
    TO_STRING_ENTRY(OSIP_NIST_STATUS_2XX_SENT_AGAIN)        
    TO_STRING_ENTRY(OSIP_NIST_STATUS_3XX_SENT)              
    TO_STRING_ENTRY(OSIP_NIST_STATUS_4XX_SENT)              
    TO_STRING_ENTRY(OSIP_NIST_STATUS_5XX_SENT)              
    TO_STRING_ENTRY(OSIP_NIST_STATUS_6XX_SENT)              
    TO_STRING_ENTRY(OSIP_NIST_STATUS_3456XX_SENT_AGAIN)     

    TO_STRING_ENTRY(OSIP_ICT_STATUS_TIMEOUT)                
    TO_STRING_ENTRY(OSIP_NICT_STATUS_TIMEOUT)               
    default:
        std::cout << "wrong type:" << type << std::endl;
        break;
    }

    return s;
}

std::string transport_error_to_string(int type)
{
    std::string s;
    switch (type) {
    TO_STRING_ENTRY(OSIP_ICT_TRANSPORT_ERROR)
    TO_STRING_ENTRY(OSIP_IST_TRANSPORT_ERROR)
    TO_STRING_ENTRY(OSIP_NICT_TRANSPORT_ERROR)
    TO_STRING_ENTRY(OSIP_NIST_TRANSPORT_ERROR)
    default:
        std::cout << "wrong type:" << type << std::endl;
        break;
    }

    return s;
}
