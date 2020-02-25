function login() {
	var acc = document.getElementById("ACC");
    	var pwd = document.getElementById("PWD");
 
    	if (acc.value == "") 
    	{
        	alert("用户名不能为空");
 		return false;
    	}
	else if (pwd.value  == "") 
	{
        	alert("密码不能为空");
		return false;
	} 
	var xmlhttp;
	if (window.XMLHttpRequest)
	{
		//  IE7+, Firefox, Chrome, Opera, Safari 浏览器执行代码
		xmlhttp=new XMLHttpRequest();
	}
	else
	{
		// IE6, IE5 浏览器执行代码
		xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
	}
	xmlhttp.onreadystatechange=function()
	{
		if (xmlhttp.readyState==4 && xmlhttp.status==200)   //readyState:0~4   4表示请求已完成并且响应已就绪
		{
			//document.getElementById("myDiv").innerHTML=xmlhttp.responseText;
			if(xmlhttp.responseText == "not exist")
			{
				alert("该用户不存在！");
			}
			else if(xmlhttp.responseText == "password error")
			{
				alert("密码错误！");
			}
			else
			{
				alert("登录成功！");
				location.href = "home.html&"+acc.value;
			}
		}
	}
	xmlhttp.open("POST","/user/loginin",true);
	xmlhttp.send("ACC="+acc.value+"&PWD="+pwd.value);

    	return true;
}

function register() {
	var acc = document.getElementById("ACC");
	var pwd = document.getElementById("PWD");
	var pwd2 = document.getElementById("PWD2");
	var pin = document.getElementById("PIN");
	var name = document.getElementById("NICK");

	if (acc.value == "") 
	{
	 	alert("用户名不能为空");
		return false;
	} 
	else if (pwd.value  == "" || pwd2.value  == "") 
	{
		alert("密码不能为空");
		return false;
	} 
	else if (pin.value  == "") 
	{
		alert("验证码不能为空");
		return false;
	}
	else if (pwd.value  !=  pwd2.value) 
	{
		alert("密码输入不一致！");
		return false;
	} 
	else if (name.value  == "") 
	{
        	alert("昵称不能为空");
		return false;
    	}
	var xmlhttp;
	if (window.XMLHttpRequest)
	{
		//  IE7+, Firefox, Chrome, Opera, Safari 浏览器执行代码
		xmlhttp=new XMLHttpRequest();
	}
	else
	{
		// IE6, IE5 浏览器执行代码
		xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
	}
	xmlhttp.onreadystatechange=function()
	{
		if (xmlhttp.readyState==4 && xmlhttp.status==200)
		{
			if(xmlhttp.responseText == "ok")
			{
				alert("注册成功！");
				location.href = "index.html";
			}
			else if(xmlhttp.responseText == "exist")
			{
				alert("该用户已存在！")
			}	
		}
	}
	xmlhttp.open("POST","/user/register",true);
	xmlhttp.send("NICK="+name.value+"&ACC="+acc.value+"&PWD="+pwd.value+"&PIN="+pin.value);
    	return true;
}

function resetpwd() {
 
	var acc = document.getElementById("ACC");
	var pwd = document.getElementById("PWD");
	var pwd2 = document.getElementById("PWD2");
	var pin = document.getElementById("PIN");

	if (acc.value == "") 
	{
	 	alert("用户名不能为空");
		return false;
	} 
	else if (pwd.value  == "" || pwd2.value  == "") 
	{
		alert("密码不能为空");
		return false;
	} 
	else if (pin.value  == "") 
	{
		alert("验证码不能为空");
		return false;
	}
	else if (pwd.value  !=  pwd2.value) 
	{
		alert("密码输入不一致！");
		return false;
	} 
	var xmlhttp;
	if (window.XMLHttpRequest)
	{
		//  IE7+, Firefox, Chrome, Opera, Safari 浏览器执行代码
		xmlhttp=new XMLHttpRequest();
	}
	else
	{
		// IE6, IE5 浏览器执行代码
		xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
	}
	xmlhttp.onreadystatechange=function()
	{
		if (xmlhttp.readyState==4 && xmlhttp.status==200)
		{ls
			if(xmlhttp.responseText == "ok")
			{
				alert("重置成功！");
			}
			else if(xmlhttp.responseText == "not exist")
			{
				alert("用户不存在！")
			}
			else if(xmlhttp.responseText == "pin error")
			{
				alert("识别码错误！")
			}	
		}
	}
	xmlhttp.open("POST","/user/resetpwd",true);
	xmlhttp.send("ACC="+acc.value+"&PWD="+pwd.value+"&PIN="+pin.value);
    	return true;
}
