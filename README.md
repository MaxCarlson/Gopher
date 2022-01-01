# Gopher
Gopher is a [GTP](https://senseis.xmp.net/?GoTextProtocol) compatible Go game engine. It uses a deep convolutional neural network, [GoNet](https://github.com/MaxCarlson/GoNet), along with a tree search in deciding where it would like to move. GoNet trains on expert moves and attempts to predict both where an expert would move and how likely the player is to win, based on a given board state. Gopher incorporates this information into a tree search, replacing the old method of random (Monte Carlo) playouts at leaf nodes. 

## Network
More information about the network being used can be found [here](https://github.com/MaxCarlson/GoNet). Example networks are included under [Gopher/models](https://github.com/MaxCarlson/Gopher/tree/master/Gopher/models), and network path can be changed from the command line. At the moment, networks must be generated in the same format as GoNet describes, and saved as a CNTK model. 

## Performance
Gopher plays better than most beginners, but is still not all that strong. I haven't had time to setup an instance of Gopher on one of the Go servers, though I may do that once Gopher is stronger than I am. At the moment Gopher is being played against other versions of itself to determine what features get to survive.

## Arguments
Gopher is under active development, and this list of arguments is changing frequently. 

* `--playouts, -p   (int)`: number of playouts to perform during each search
* `--resign, -r   (float)`: threshold at which the Gopher will resign. Ex. -r 0.05 will have Gopher resign if the network thinks it has <= 5% win chance
* `--val, -v       (bool)`: validation mode. This is used internally for play testing the engine against others. 
* `--vmax, -m:      (int)`: a cap on the maximum number of moves per game while in validation mode. 
* `--noise          (int)`: adds noise into move selection for the first n moves. Moves will be selected preportionally based on how much the network likes them.
* `--net, -n     (string)`: change the default path to the network ('Gopher\models\GoNet.dnn')
* `--history, -h    (int)`: set the number of past+present feature planes the network should recieve


## Under Construction
Once the Net is integrated and everything is up and running again I'll add some pictures/video of Gophers use and play.
