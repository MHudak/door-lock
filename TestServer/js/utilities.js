            var PASSWORD_LENGTH = 8;
            var username = "";
            var password = "";
            var seed = null;
            var resaltPassword = true;
                    
            //random number generator
            var currentRandomNumberSeed = 0;
            function getRandomInt(){
                currentRandomNumberSeed = currentRandomNumberSeed & 0xFFF;
                currentRandomNumberSeed = (214013 * currentRandomNumberSeed + 2531011)%16777216;
                return currentRandomNumberSeed;
            }
            function calculatePassword(password){
                currentRandomNumberSeed = parseInt(localStorage.getItem("seed"));
                var saltedPW = "";
                var i;
                for(i = 0; i < password.length && i < (PASSWORD_LENGTH*3); i++){
                    saltedPW += format(((password.charCodeAt(i) + getRandomInt()) % 256), 3);
                }
                while(saltedPW.length < (PASSWORD_LENGTH*3)){
                    saltedPW += format(((getRandomInt()) % 256), 3);
                }
                return saltedPW;
            }       

            //url formatter     
            function format(value, length){
                var returnVal = new Array();
                while(length-- > 0){
                    if(value > 0){
                        returnVal[length] = value%10;
                        value = Math.floor(value/10);
                    }else{
                        returnVal[length] = 0;
                    }
                }
                return returnVal.join("");
            }
