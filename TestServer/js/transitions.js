
    $( document ).ready(function() {
        $(".smess").hide();
        $(".fmess").hide();
        checkSeed();
    });

    //check for seed value stored on client's computer
    function checkSeed() {
        seed=localStorage.getItem("seed");
        if (seed != null) {
            currentRandomNumberSeed = parseInt(seed);
            $(":mobile-pagecontainer").pagecontainer( "change", "#login", {transition: "flip"});
        } else {
            // if no seed exists, prompt user to enter their given seed value
            $(":mobile-pagecontainer").pagecontainer( "change", "#enter-seed", {transition: "flip"});
        }
        $(".fouc").removeClass("fouc");
    }   

    $("#enter-seed").submit(function(e)
    {
        seed = e.target["user-seed"].value;

        if(!!seed){
            localStorage.setItem("seed", seed);
            currentRandomNumberSeed = parseInt(localStorage.getItem("seed"));
            $(":mobile-pagecontainer").pagecontainer( "change", "#login", {transition: "flip"}); 
        }
        return false;
    });
    $("#login").submit(function(e){
        $(".fmess").hide();
        username = e.target["user-id"].value;
        password = e.target["user-password"].value;
        if(resaltPassword){
            resaltPassword = false;
            var saltedPW = calculatePassword(password);
        }
        var formURL = "/?uname=" + username + "&pw=" + saltedPW;
        $.ajax(
        {
            url : formURL,
            type: "GET",

            // Successful seed submission, take user to login page
            success:function(data, textStatus, jqXHR) 
            {
                resaltPassword = true;
                $(":mobile-pagecontainer").pagecontainer( "change", "#door-lock-unlock", {transition: "flip"});
                localStorage.setItem("seed", currentRandomNumberSeed);
            },
            //TODO implement better failure notification transition
            //failed seed submission, take to failure notification page
            error: function(jqXHR, textStatus, errorThrown) 
            {
                resaltPassword = true;
                $(".error-message").show();
            }
        });
    });

    $("#door-lock-unlock").submit(function(e){
        $(".smess").hide();
        $(".fmess").hide();
        if(resaltPassword){
            resaltPassword = false;
            var saltedPW = calculatePassword(password);
        }
        var formURL = "/?" + "cmd=" + e.target["select-based-flipswitch"].value  + "&uname=" + username + "&pw=" + saltedPW;

        $.ajax(
        {
            url : formURL,
            type: "GET",
            
            success:function(data, textStatus, jqXHR) 
            {
                resaltPassword = true;
                $(".smess").show();
                localStorage.setItem("seed", currentRandomNumberSeed);
            },
            error: function(jqXHR, textStatus, errorThrown) 
            {
                resaltPassword = true;
                $(".fmess").show();
            }
        });
        return false;
    });