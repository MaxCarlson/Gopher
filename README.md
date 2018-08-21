# Gopher
Gopher is a [GTP](https://senseis.xmp.net/?GoTextProtocol) compatiable Go game engine. It uses a deep residual neural network, [GoNet](https://github.com/MaxCarlson/GoNet), along with a tree search in deciding where it would like to move. GoNet trains on expert moves and attempts to predict both where an expert would move and how likely the player is to win, based on a given board state. Gopher incorporates this information into a tree search, replacing the old method of random playouts at leaf nodes. 

## Under Construction
Once the Net is integrated and everything is up and running again I'll add some pictures/video of Gophers use and play.
