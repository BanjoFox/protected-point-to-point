<form id="login" method="post" action="<?php echo $PHP_SELF ?>">
<p class="logintext">Username: <input class="theInput" name="usernamef" type="text" /></p>
<p class="logintext">Password: <input class="theInput" name="passwordf" type="password" /></p>
<button class="Submit" name="Submit" value="Submit" type="submit">Submit</button>
</form>
<?php
$usernamef = $_POST['usernamef'];
$passwordf = $_POST['passwordf'];
$username='Pete';
$passwrd='password';

if ($_POST[Submit]=="Submit"){
        if ($usernamef==$username && $passwordf==$passwrd){
                    echo "WHAT YOU ARE HIDING";}
else {echo "Username or Password Incorrect"; }
} ?>
