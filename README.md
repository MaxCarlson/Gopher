# Gopher
Gopher is a [GTP](https://senseis.xmp.net/?GoTextProtocol) compatiable Go game engine. It uses a deep residual neural network, [GoNet](https://github.com/MaxCarlson/GoNet), along with a tree search in deciding where it would like to move. GoNet trains on expert moves and attempts to predict both where an expert would move and how likely the player is to win, based on a given board state. Gopher incorporates this information into a tree search, replacing the old method of random (Monte Carlo) playouts at leaf nodes. 

## Network
More information about the network being used can be found [here](https://github.com/MaxCarlson/GoNet). Example networks are included under [Gopher/models](https://github.com/MaxCarlson/Gopher/tree/master/Gopher/models), and network path can be changed from the command line. At the moment, networks must be generated in the same format as GoNet describes, and saved as a CNTK model. 

## Under Construction
Once the Net is integrated and everything is up and running again I'll add some pictures/video of Gophers use and play.
